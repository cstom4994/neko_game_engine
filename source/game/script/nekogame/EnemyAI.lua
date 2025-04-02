local nn = hot_require("nn")

class "EnemyAI"

function EnemyAI:new(x, y)

    -- 敌人属性
    self.x = x or math.random(-100, 100)
    self.y = y or math.random(-100, 100)
    self.health = 100
    self.attack_cooldown = 0
    self.random_move_timer = 0

    -- 玩家位置
    self.player_x = 0
    self.player_y = 0

    -- 神经网络结构
    self.nn = nn:new_network({6, 12, 8, 3}, 0.1, 0.9) -- 输入6，隐藏层12和8，输出3

    -- 经验回放
    self.memory = {}
    self.memory_size = 200
    self.batch_size = 32

    -- 训练参数
    self.exploration_rate = 0.5
    self.min_exploration = 0.05
    self.exploration_decay = 0.9998

    -- 新增参数
    self.last_distance = 100
    self.safe_distance = 15
    self.optimal_attack_distance = 8
end

function EnemyAI:update_player_position(x, y)
    self.player_x = x
    self.player_y = y
end

function EnemyAI:distance_to_player()
    local dx = self.player_x - self.x
    local dy = self.player_y - self.y

    -- 处理极大数值情况
    if math.abs(dx) > 1e6 or math.abs(dy) > 1e6 then
        return math.huge -- 返回无穷大
    end

    return math.sqrt(dx * dx + dy * dy)
end

function EnemyAI:random_move()
    self.random_move_timer = self.random_move_timer - 1
    if self.random_move_timer <= 0 then
        self.random_move_timer = math.random(10, 30)
        return math.random(-1, 1), math.random(-1, 1)
    end
    return 0, 0
end

function EnemyAI:get_nn_action()
    local random_factor = math.random() * 2 - 1

    -- 使用sigmoid函数归一化坐标到0-1范围
    local function normalize(v)
        return 1 / (1 + math.exp(-v * 0.01)) -- 缩放因子0.01使大多数值在合理范围内
    end

    local input = {normalize(self.player_x), normalize(self.player_y), normalize(self.x), normalize(self.y),
                   self.health / 100, random_factor}
    local output = self.nn:forward(input)
    return output[1] * 2 - 1, output[2] * 2 - 1, output[3] -- 将输出从[0,1]映射到[-1,1]
end

function EnemyAI:calculate_reward(distance, attacked)
    local reward = 0

    -- 距离变化奖励
    local distance_change = self.last_distance - distance
    reward = reward + distance_change * 2

    -- 绝对距离奖励 (S型曲线)
    local distance_factor = 1 / (1 + math.exp(0.5 * (distance - self.optimal_attack_distance)))
    reward = reward + distance_factor * 30

    -- 攻击奖励
    if attacked then
        reward = reward + 100 * (1 - distance / 30)
    end

    -- 危险惩罚
    if distance < 5 then
        reward = reward - 40 * (6 - distance)
    end

    -- 健康状态奖励
    reward = reward + self.health * 0.2

    self.last_distance = distance
    return reward
end

function EnemyAI:decide_action()
    local distance = self:distance_to_player()

    if math.random() < self.exploration_rate then
        if distance > self.safe_distance then
            -- 计算归一化方向向量
            local dx = self.player_x - self.x
            local dy = self.player_y - self.y
            local length = math.sqrt(dx * dx + dy * dy)

            -- 防止除以零
            if length > 0 then
                dx = dx / length
                dy = dy / length
            else
                dx, dy = 1, 0 -- 默认向右移动
            end

            local rnd = math.random() * 0.5
            return dx + rnd, dy + rnd, 0
        else
            return math.random() * 0.4 - 0.2, math.random() * 0.4 - 0.2, 0
        end
    else
        return self:get_nn_action()
    end
end

function EnemyAI:update()
    -- 冷却时间更新
    if self.attack_cooldown > 0 then
        self.attack_cooldown = self.attack_cooldown - 1
    end

    -- 获取当前距离
    local distance = self:distance_to_player()

    -- 决定行动
    local dx, dy, attack_will = self:decide_action()

    dx = type(dx) == "table" and dx[1] or dx
    dy = type(dy) == "table" and dy[1] or dy

    -- 移动处理
    local move_x, move_y = 0, 0
    local move_speed = 1

    if distance > 20 then
        move_x = dx * 1.5
        move_y = dy * 1.5
    elseif distance < 8 then
        move_x = dx * 0.6
        move_y = dy * 0.6
    else
        move_x = dx
        move_y = dy
    end

    -- 应用移动
    self.x = self.x + move_x * move_speed
    self.y = self.y + move_y * move_speed

    -- 攻击逻辑
    local attacked = false
    if distance < 30 and attack_will > 0.5 and self.attack_cooldown == 0 then
        attacked = true
        self.attack_cooldown = 10
    end

    -- 计算奖励
    local reward = self:calculate_reward(distance, attacked)

    -- 被攻击检测
    if distance < 5 and self:is_attacked_by_player() then
        self.health = self.health - 10
        reward = reward - 30
    end

    -- 死亡检测
    if self.health <= 0 then
        reward = reward - 100
    end

    -- 准备训练数据
    local expected_output = {0, 0, 0}
    local distance_x = self.player_x - self.x
    local distance_y = self.player_y - self.y

    -- 理想移动方向
    if math.abs(distance_x) > 5 then
        expected_output[1] = distance_x > 0 and 1 or -1
    end
    if math.abs(distance_y) > 5 then
        expected_output[2] = distance_y > 0 and 1 or -1
    end

    -- 理想攻击时机
    if distance < 10 and self.attack_cooldown == 0 then
        expected_output[3] = 1
    end

    -- 加入经验回放
    local input = {self.player_x / 100, self.player_y / 100, self.x / 100, self.y / 100, self.health / 100,
                   math.random() * 2 - 1}

    table.insert(self.memory, {
        input = input,
        output = expected_output,
        reward = reward
    })

    -- 限制记忆大小
    if #self.memory > self.memory_size then
        table.remove(self.memory, 1)
    end

    -- 批量训练
    if #self.memory >= self.batch_size then
        for i = 1, math.min(self.batch_size, #self.memory) do
            local sample = self.memory[math.random(1, #self.memory)]
            self.nn:learn(sample.input, sample.output)
        end
    end

    -- 衰减探索率
    self.exploration_rate = math.max(self.min_exploration, self.exploration_rate * self.exploration_decay)

    return reward
end

function EnemyAI:is_attacked_by_player()
    -- 10%概率被攻击
    return math.random() < 0.1
end

function EnemyAI:draw()
end

function EnemyAI:info()
    local distance = self:distance_to_player()
    local info = string.format("Enemy at (%.1f, %.1f) HP: %d\nDecision: Dist=%.1f\nStrategy: %s\nAttack: %s", self.x,
        self.y, self.health, distance, distance > self.safe_distance and "Approaching" or "Maneuvering",
        self.attack_cooldown > 0 and "On Cooldown" or "Ready")
    return info
end

-- 主模拟函数
local function Test()
    -- 初始化
    math.randomseed(os.time())
    local player_x, player_y = 50, 50
    local enemy = EnemyAI:new(math.random(1, 100), math.random(1, 100))

    -- 统计数据
    local stats = {
        steps = 30000,
        distances = {},
        rewards = {}
    }

    -- 模拟循环
    for step = 1, stats.steps do
        print(string.format("\n=== Step %d/%d ===", step, stats.steps))

        -- 玩家移动
        if step % 100 == 0 then
            player_x, player_y = math.random(1, 100), math.random(1, 100)
            print(string.format("Player jumped to (%d, %d)", player_x, player_y))
        else
            player_x = player_x + math.random(-1, 1)
            player_y = player_y + math.random(-1, 1)
            player_x = math.max(1, math.min(100, player_x))
            player_y = math.max(1, math.min(100, player_y))
        end

        -- 更新敌人
        enemy:update_player_position(player_x, player_y)
        local reward = enemy:update()

        -- 记录数据
        table.insert(stats.distances, enemy:distance_to_player())
        table.insert(stats.rewards, reward)

        -- 显示状态
        print(string.format("Player: (%d, %d)", player_x, player_y))
        enemy:draw()
        enemy:explain_decision()
        print(string.format("Reward: %.2f", reward))
        print(string.format("Explore: %.4f", enemy.exploration_rate))

        -- 重生逻辑
        if enemy.health <= 0 then
            print("Enemy died! Respawning...")
            enemy = EnemyAI:new(math.random(1, 100), math.random(1, 100))
        end
    end

    -- 分析结果
    local function average(t)
        local sum = 0
        for _, v in ipairs(t) do
            sum = sum + v
        end
        return sum / #t
    end

    print("\n=== 最终结果 ===")
    print(string.format("平均距离: %.2f", average(stats.distances)))
    print(string.format("平均奖励: %.2f", average(stats.rewards)))

    -- 绘制最后100步的距离趋势
    print("\nLast 100 Steps Distance Trend:")
    local last_distances = {}
    for i = math.max(1, #stats.distances - 99), #stats.distances do
        table.insert(last_distances, stats.distances[i])
    end
    draw_simple_plot(last_distances, 0, 100)
end

-- 绘制简易ASCII图表
local function draw_simple_plot(data, min_val, max_val)
    local height = 10
    local range = max_val - min_val

    for h = height, 1, -1 do
        local line = ""
        local threshold = min_val + (range * (h - 1) / height)

        for _, v in ipairs(data) do
            line = line .. (v >= threshold and "■" or " ")
        end

        print(string.format("%5.1f | ", threshold) .. line)
    end
    print("       |" .. string.rep("-", #data))
end
