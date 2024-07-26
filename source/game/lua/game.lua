local socket = require "socket"

local sock = require "lua/libs/sock"

function GetIP()
    local s = socket.udp()
    s:setpeername("192.168.1.1", 80)
    local ip, sock = s:getsockname()
    return ip
end

function CopyValues(tableFrom, tableTo)
    for key, value in pairs(tableFrom) do
        tableTo[key] = value
    end
end

local game = {}

local GameServer = {}
local GameClient = {}

local multicastAddress = "225.0.0.66" -- 随机多播地址
local startingZombieSpawnTime = 2

local gameStates = {
    CLIENT_WAIT = 'CLIENT_WAIT',
    CLIENT_PLAY = 'CLIENT_PLAY',
    INIT = 'INIT',
    WAITING = 'WAITING',
    READY = 'READY',
    PARTIAL_DEAD = 'PARTIAL_DEAD', -- todo
    GAME_END = 'GAME_END',
    SERVER_DISCONNECT = 'SERVER_DISCONNECT'
}

local gameChannels = {
    GAME_UPDATE_FROM_SERVER = "game_update_from_server",
    GAME_STATE = "game_state",
    CONNECT = "connect",
    CONFIG = "config",
    JOIN = "join",
    JOIN_RECEIVED = "join_rec",
    INPUT = "input",
    CLICK = "click",
    GAME_UPDATE_FROM_CLIENT = "game_update_from_client",
    DISCONNECT = "disconnect"
}

local advertiseVal = 0
local serverUpdateTimeVal = 0
function GameServer:update(dt)

    self.game_tick = self.game_tick + 1

    self.enetServer:update(dt)
    -- self.world:update(dt)

    serverUpdateTimeVal = serverUpdateTimeVal + dt

    if (serverUpdateTimeVal >= self.updateTime) then
        serverUpdateTimeVal = serverUpdateTimeVal - self.updateTime
        self:sendToAll(gameChannels.GAME_UPDATE_FROM_SERVER, {
            -- players = self.players,
            -- bullets = self.bullets,
            -- zombies = self.zombies

            -- world = inspect(self.world)
        })
    end

    if self.currentGameState ~= self.states.READY then
        advertiseVal = advertiseVal + dt
        if advertiseVal >= self.advertiseTime then
            advertiseVal = advertiseVal - self.advertiseTime
            print('advertising')
            self:advertise()
        end
    end

    -- cleanup
    if self.currentGameState == self.states.SERVER_DISCONNECT then

    end

    if self.currentGameState == self.states.GAME_END then

    end

    if self.currentGameState == self.states.READY then

    end
end

function GameServer:sendGameState()
    self:sendToAll(gameChannels.GAME_STATE, {
        currentGameState = self.currentGameState
    })
end

function GameServer:sendToAll(channel, data)
    self.enetServer:sendToAll(channel, data)
end

function GameServer:advertise()
    self.adServer:sendto("game", multicastAddress, 11111)
end

function GameServer:destroy()
    self.enetServer:destroy()
end

function GameServer:setupCallbacks()
    -- Called when someone connects to the server
    self.enetServer:on("connect", function(data, client)
        print('Client joined : ', client:getConnectId())
        self.currentGameState = self.states.INIT
        -- the only reason we cannot use getConnectId itself as
        -- the unique id is because on disconnect, getConnectId
        -- returns nil
        client.uniqueID = client:getConnectId()
        client:send(gameChannels.CONFIG, {
            -- updateTime = self.updateTime,
            currentGameState = self.currentGameState
        })
    end)

    self.enetServer:on(gameChannels.JOIN, function(data, client)
        print('Join received from client : ', client.uniqueID)
        -- local newPlayer = player.spawn(client:getIndex(), client.uniqueID)
        -- self.players[newPlayer.id] = newPlayer
        -- client:send(gameChannels.JOIN_RECEIVED, {
        --     player = newPlayer
        -- })
        -- print('Total number of players : ', GetMapSize(self.players))
    end)

    self.enetServer:on(gameChannels.INPUT, function(data, client)

    end)

    self.enetServer:on(gameChannels.CLICK, function(data, client)
        print('click received from client : ', client)
    end)

    self.enetServer:on(gameChannels.GAME_UPDATE_FROM_CLIENT, function(data, client)
        print('player_update received from : ', client)
    end)

    self.enetServer:on(gameChannels.DISCONNECT, function(data, client)
        print('client disconnected! : ', inspect(client))
    end)
end

function GameClient:setupConnectionToServer(hostAddress)
    if not self.client then
        self.hostIPFound = true
        self.client = sock.newClient(hostAddress, self.port)
        -- self.client:setSerialization(bitser.dumps, bitser.loads)

        -- Called when a connection is made to the server
        self.client:on(gameChannels.CONNECT, function(data, server)
            self.client:send("join", {
                -- timestamp = love.timer.getTime()
            })
        end)

        self.client:on(gameChannels.CONFIG, function(data, server)
            print('updateTime frequency received from server : ', data.updateTime)
            -- self.updateTime = data.updateTime
            self.currentGameState = data.currentGameState
        end)

        self.client:on(gameChannels.DISCONNECT, function(data, server)
            print('Disconnected from server')
            -- love.event.quit()
        end)

        self.client:on(gameChannels.JOIN_RECEIVED, function(data)
            self.player = data.player
            print('my player_id : ', self.player.id)
            -- love.window.setTitle(self.player.name)
        end)

        self.client:on(gameChannels.GAME_STATE, function(data)
            self.currentGameState = data.currentGameState
            print('currentGameState now : ', self.currentGameState)
            if self.currentGameState == self.states.READY then
                -- self:resetGame()
            end
        end)

        self.client:on(gameChannels.GAME_UPDATE_FROM_SERVER, function(data)
            print('received game update from server')
        end)

        self.client:connect()
        print('connected to server : ', hostAddress)
    end
end

function GameClient:searchForServer()
    print('searching for server')
    local data, ip, _ = self.adClient:receivefrom()
    -- print(adClient:receivefrom())
    if data == "game" then
        -- fresh start
        -- self:resetGame()
        -- DestroyWorld(self.world)
        self:setupConnectionToServer(ip)
        self.adClient:close() -- very important
    end
end

local searchVal = 0
local updateTimeVal = 0
function GameClient:update(dt)
    -- self.world:update(dt)
    if self.client then
        self.client:update()
    end

    if not self.hostIPFound then
        searchVal = searchVal + dt
        if searchVal >= self.searchTime then
            searchVal = searchVal - self.searchTime
            self:searchForServer()
        end
    end

    -- if self.currentGameState == self.states.GAME_END or self.currentGameState == self.states.SERVER_DISCONNECT then
    --     ResetValuesOnEnd(self)
    --     DestroyWorld(self.world)
    --     return
    -- end

    updateTimeVal = updateTimeVal + dt
    if self.client and updateTimeVal >= self.updateTime then
        updateTimeVal = updateTimeVal - self.updateTime
        self.client:send(gameChannels.GAME_UPDATE_FROM_CLIENT, {
            -- playerID = self.player.id,
            -- playerX = self.player.x,
            -- playerY = self.player.y,
            -- playerAngle = self.player.angle
        })
    end

end

game.newClient = function(port)

    local adClient = assert(socket.udp4())
    assert(adClient:setoption("reuseport", true))
    assert(adClient:setsockname("*", 11111))
    assert(adClient:setoption("ip-add-membership", {
        multiaddr = multicastAddress,
        interface = "*"
    }))
    adClient:settimeout(0)

    local gc = setmetatable({
        client = nil,
        hostIPFound = false,
        currentGameState = gameStates.CLIENT_WAIT,

        port = port,
        -- world = SetupWorld(),
        angles = {},
        angleIndex = 0, -- the index at which the angle is read
        angleLag = 10,
        anglesMax = 1200,

        players = {},
        playersClientCopy = {},
        player = {},

        -- colliders
        playerCollider = {},
        zombieCollider = {},
        bulletCollider = {},

        startingZombieSpawnTime = startingZombieSpawnTime,
        maxTime = startingZombieSpawnTime,
        timer = startingZombieSpawnTime,

        zombies = {},
        zombieNum = 0,

        bullets = {},
        bulletNum = 0,
        bulletFiringRate = 0.3, -- one bullet per 0.5 seconds

        -- updateTime = 1 / 30,
        updateTime = 4,

        searchTime = 1,
        adClient = adClient,

        states = gameStates
    }, {
        __index = GameClient
    })

    return gc
end

game.newServer = function(ip, port)

    local adServer = assert(socket.udp4())
    adServer:settimeout(0)

    local enetServer = sock.newServer(ip, port)
    -- enetServer:setSerialization(bitser.dumps, bitser.loads)

    local gs = setmetatable({
        enetServer = enetServer,
        -- world = SetupWorld(),
        players = {}, -- player.id -> player{}
        players_ready = {}, -- player.id -> 1
        players_in_game = {}, -- player.id -> 1

        -- colliders
        playerCollider = {},
        zombieCollider = {},
        bulletCollider = {},

        zombies = {},
        zombieNum = 0,

        bullets = {},
        bulletNum = 0,

        game_tick = 0,

        world = nil,
        b2 = nil,
        tilemap = nil,

        startingZombieSpawnTime = startingZombieSpawnTime,
        maxTime = startingZombieSpawnTime,
        timer = startingZombieSpawnTime,

        -- updateTime = 1 / 30,
        updateTime = 4,

        advertiseTime = 2,
        adServer = adServer,

        states = gameStates,
        currentGameState = gameStates.INIT

    }, {
        __index = GameServer
    })

    gs:setupCallbacks()
    return gs
end

return game
