-- 矩阵库
-- ====================
-- 需要的所有矩阵数学
local matrix_utilities = {
    summate = function(array)
        local total = 0
        for i = 1, #array do
            total = total + array[i]
        end
        return total
    end,
    diff = function(ar1, ar2, abs)
        local error_array = {}
        for i = 1, #ar1[1] do
            error_array[i] = ar2[1][i] - ar1[1][i]
            if abs == true then
                error_array[i] = math.abs(error_array[i])
            end
        end
        return error_array
    end,
    average = function(input_matrix)
        local final_array = 0
        for i = 1, #input_matrix do
            final_array = final_array + input_matrix[i]
        end
        return final_array / #input_matrix
    end,

    dot = function(inp1, inp2)
        local total = 0
        if (#inp1 ~= #inp2) then
            return false
        end
        for i = 1, #inp1 do
            total = total + (inp1[i] * inp2[i])
        end
        return total
    end,

    random = function(width, height)
        local final = {}
        for i = 1, height do
            local temp = {}
            for j = 1, width do
                temp[j] = math.random() - 0.5
            end
            table.insert(final, temp)
        end
        return final
    end,
    zeros = function(width, height)
        local final = {}
        for i = 1, height do
            local temp = {}
            for j = 1, width do
                temp[j] = 0
            end
            table.insert(final, temp)
        end
        return final
    end,

    transpose = function(input_matrix)
        local output = {}
        for i = 1, #input_matrix do
            for j = 1, #input_matrix[i] do
                output[j] = output[j] or {}
                output[j][i] = input_matrix[i][j]
            end
        end
        return output
    end,

    multiply = function(matrix1, matrix2)
        -- init and check
        local output = {}
        if m.len(matrix1) ~= m.height(matrix2) then
            return false
        end

        for i = 1, m.height(matrix1) do
            for j = 1, m.len(matrix2) do
                output[i] = output[i] or {}
                local final = 0

                if m.len(matrix1) == 1 then
                    for c = 1, m.len(matrix1) do
                        final = final + (matrix1[c][1] * matrix2[i][c])
                    end
                elseif m.len(matrix2) == 1 then
                    for c = 1, m.len(matrix2) do
                        final = final + (matrix1[c][j] * matrix2[1][c])
                    end
                else
                    for c = 1, m.len(matrix1) do
                        final = final + (matrix1[c][j] * matrix2[i][c])
                    end
                end

                output[i][j] = final
            end
        end
        return output
    end,

    height = function(x)
        if type(x) == "table" then
            return #x
        else
            return 1
        end
    end,

    len = function(x)
        if type(x[1]) == "table" then
            return #x[1]
        else
            return 1
        end
    end,

    cost = function(input, expected_output)
        local output = 0
        for i = 1, #input do
            output = output + math.pow(input[i] - expected_output[i], 2)
        end
        return output / 2
    end
}

-- ====================
-- 所有神经网络逻辑
class "nn"

-- 创建新的网络结构
-- 注意突触在 0 和 1 之间随机化
-- 节点不需要初始化
function nn:new_network(structure, rate, momentum)
    local nn = {}
    setmetatable(nn, self)

    -- 为新网络设置值
    self.__index = self
    self.structure = structure
    self.synapses = nn:create_synapse_structure(structure)
    self.learning_rate = rate or 0.2
    self.momentum_multiplier = momentum or 0

    -- 零初始化
    self.gamma = {}
    self.output = {}
    self.input = {}
    self.exp_out = {}
    self.nodes = {}

    return nn
end

-- Sigmoid/阈值函数
-- 函数近似非线性
function nn:sigmoid(x)
    return 1 / (1 + math.exp(-1 * x))
end

function nn:derivative_sigmoid(x)
    return self:sigmoid(x) * (1 - self:sigmoid(x))
end

function nn:inverse_sig(x)
    return -math.log((1 / x) - 1)
end

-- 非常基本的阈值函数
function nn:relu(x)
    if x < 0 then
        return 0
    else
        return x
    end
end

-- 均方误差算法
-- 取两个向量 计算每个相邻值之间的平均平方差
function nn:MSE(actual_value, expected_value)
    local total = 0
    for i = 1, #actual_value do
        total = total + (math.pow(actual_value[i] - expected_value[i], 2) / 2)
    end
    return total
end

-- 前向传播
-- 接受向量输入 在网络上执行传递并返回输出层
function nn:forward(input)
    -- set first node layer values to our vector input
    self.nodes[1] = input
    -- for each layer of synapses
    for s = 1, #self.synapses do
        self.nodes[s + 1] = {}
        for i = 1, #self.synapses[s] do
            self.nodes[s + 1][i] = nn:sigmoid(matrix_utilities.dot(self.nodes[s], self.synapses[s][i]))
        end
    end
    return self.nodes[#self.nodes]
end

-- 反向传播网络的最终突触层 (输出层)
-- 应始终首先计算该层
function nn:backpropagate_output_layer(actual_output, expected_output, ln_rate)
    local err_num = {}
    self.deltas = self.deltas or {}
    self.deltas[#self.synapses] = self.deltas[#self.synapses] or {}
    self.gamma[#self.nodes] = {}

    -- For num of outputs
    for i = 1, #self.nodes[#self.nodes] do
        err_num[i] = actual_output[i] - expected_output[i]
    end

    for i = 1, #self.nodes[#self.nodes] do
        self.gamma[#self.nodes][i] = err_num[i] * self:derivative_sigmoid(self:inverse_sig(actual_output[i]))
    end

    -- update self.deltas
    for i = 1, #self.nodes[#self.nodes] do
        self.deltas[#self.synapses][i] = self.deltas[#self.synapses][i] or {}
        for j = 1, #self.nodes[#self.nodes - 1] do
            local previous_weights_for_momentum = self.deltas[#self.synapses][i][j] or 0

            self.deltas[#self.synapses][i][j] = self.gamma[#self.nodes][i] * (self.nodes[#self.nodes - 1][j]) +
                                                    previous_weights_for_momentum * self.momentum_multiplier
        end
    end
end

-- 反向传播网络的隐藏突触层
-- 应在输出层反向传播之后计算
function nn:backpropagate_hidden_layers(L, ln_rate)
    -- current layer
    self.gamma[L + 1] = {}
    self.deltas[L] = self.deltas[L] or {}

    for i = 1, #self.nodes[L + 1] do
        self.gamma[L + 1][i] = 0

        -- self.gamma forward
        for j = 1, #self.gamma[L + 2] do
            self.gamma[L + 1][i] = self.gamma[L + 1][i] + (self.gamma[L + 2][j] * self.synapses[L + 1][j][i])
        end

        -- As nodes contain their values post-sigmoid, we have to
        -- invert the sigmoid function before deriving it
        self.gamma[L + 1][i] = self.gamma[L + 1][i] * self:derivative_sigmoid(self:inverse_sig(self.nodes[L + 1][i]))
    end

    -- update self.deltas
    for i = 1, #self.nodes[L + 1] do
        self.deltas[L][i] = self.deltas[L][i] or {}

        -- for every synapse
        for j = 1, #self.nodes[L] do
            local previous_weights_for_momentum = self.deltas[L][i][j] or 0
            self.deltas[L][i][j] = self.gamma[L + 1][i] * (self.nodes[L][j]) + previous_weights_for_momentum *
                                       self.momentum_multiplier
        end
    end

    return self.deltas
end

-- 2个三维矩阵的加法
function nn:add_weights(m1, m2)
    local output = {}
    for i = 1, #m1 do
        output[i] = {}
        for j = 1, #m1[i] do
            output[i][j] = {}
            for k = 1, #m1[i][j] do
                output[i][j][k] = m1[i][j][k] + m2[i][j][k]
            end
        end
    end
    return output
end

-- 2个三维矩阵的减法
function nn:subtract_weights(m1, m2)
    local output = {}
    for i = 1, #m1 do
        output[i] = {}
        for j = 1, #m1[i] do
            output[i][j] = {}
            for k = 1, #m1[i][j] do
                output[i][j][k] = m1[i][j][k] - m2[i][j][k] * self.learning_rate
            end
        end
    end
    return output
end

function nn:create_synapse_structure(struct)
    local newstruct = {}
    for w = 1, #struct - 1 do
        newstruct[w] = matrix_utilities.random(struct[w], struct[w + 1])
    end
    return newstruct
end

function nn:learn(input, expected_output)
    local real_output = self:forward(input)

    -- 反向传播最后一层 (output layer)
    self:backpropagate_output_layer(real_output, expected_output, self.learning_rate)

    -- 所有隐藏层的反向传播
    for layer = #self.synapses - 1, 1, -1 do
        self:backpropagate_hidden_layers(layer, self.learning_rate)
    end

    self.synapses = self:subtract_weights(self.synapses, self.deltas)
    return self:MSE(real_output, expected_output)
end

function nn:inspect()
    local report = {}

    table.insert(report, "===== 神经网络 =====")
    table.insert(report, "")

    -- 网络基本信息
    table.insert(report, "1. 网络基本信息:")
    table.insert(report, string.format("   - 学习率: %.4f", self.learning_rate))
    table.insert(report, string.format("   - 动量系数: %.4f", self.momentum_multiplier))
    table.insert(report, string.format("   - 网络层数: %d (输入层 + %d隐藏层 + 输出层)", #self.structure,
        #self.structure - 2))
    table.insert(report, "")

    -- 网络层结构
    table.insert(report, "2. 网络层结构:")
    for i, v in ipairs(self.structure) do
        local layer_type
        if i == 1 then
            layer_type = "输入层"
        elseif i == #self.structure then
            layer_type = "输出层"
        else
            layer_type = "隐藏层 " .. (i - 1)
        end
        table.insert(report, string.format("   - %s: %d 个神经元", layer_type, v))
    end
    table.insert(report, "")

    -- 权重信息
    table.insert(report, "3. 突触权重信息:")
    for i, layer in ipairs(self.synapses) do
        table.insert(report, string.format("   - 突触层 %d (连接层 %d → 层 %d):", i, i, i + 1))
        table.insert(report, string.format("     - 权重矩阵尺寸: %d × %d", #layer[1], #layer))

        -- 计算权重统计信息
        local min, max, avg = math.huge, -math.huge, 0
        local count = 0
        for _, neuron in ipairs(layer) do
            for _, weight in ipairs(neuron) do
                min = math.min(min, weight)
                max = math.max(max, weight)
                avg = avg + weight
                count = count + 1
            end
        end
        avg = avg / count

        table.insert(report, string.format("     - 权重范围: [%.4f, %.4f]", min, max))
        table.insert(report, string.format("     - 平均权重: %.4f", avg))
    end
    table.insert(report, "")

    -- 节点信息
    if self.nodes and #self.nodes > 0 then
        table.insert(report, "4. 最近一次前向传播节点值:")
        for i, layer in ipairs(self.nodes) do
            local layer_type
            if i == 1 then
                layer_type = "输入层"
            elseif i == #self.nodes then
                layer_type = "输出层"
            else
                layer_type = "隐藏层 " .. (i - 1)
            end

            table.insert(report, string.format("   - %s:", layer_type))
            for j, val in ipairs(layer) do
                table.insert(report, string.format("     - 节点 %d: %.4f", j, val))
            end
        end
        table.insert(report, "")
    end

    -- 梯度信息
    if self.gamma and #self.gamma > 0 then
        table.insert(report, "5. 最近一次反向传播梯度信息:")
        for i = #self.gamma, 1, -1 do
            local layer_type
            if i == #self.gamma then
                layer_type = "输出层"
            elseif i == 1 then
                layer_type = "输入层后梯度"
            else
                layer_type = "隐藏层 " .. (i - 1)
            end

            table.insert(report, string.format("   - %s:", layer_type))
            if self.gamma[i] then
                for j, val in ipairs(self.gamma[i]) do
                    table.insert(report, string.format("     - 节点 %d 梯度: %.4f", j, val))
                end
            end
        end
    end

    return table.concat(report, "\n")
end
