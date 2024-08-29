ns.player = {
    pool = {},
    current_player = ng.entity_nil
}

local Player = ns.player

function ns.player.update_all()
    local cam = ns.camera.get_current_camera()
    if cam == ng.entity_nil then
        print("player no camera?")
        return
    end
    local pl = Player.current_player
    local cam_pos = ns.transform.get_world_position(cam)
    local pl_pos = ns.transform.get_world_position(pl)
    local dt = ns.timing.instance.dt

    local blend = 1 - 0.85 ^ (dt * 40)
    cam_pos.x = lerp(cam_pos.x, pl_pos.x, blend)
    cam_pos.y = lerp(cam_pos.y, pl_pos.y, blend)
    -- camera.scale = lerp(camera.scale, 4, dt ^ 0.8)

    ns.transform.set_position(cam, cam_pos)

end

function ns.player.draw_ui()

end

function ns.player.has(ent)
    print("ns.player.has " .. ent.id)
    return (Player.pool[ent] ~= nil)
end

function ns.player.add(ent)
    print("ns.player.add " .. ent.id)
    assert(Player.current_player == ng.entity_nil)
    Player.current_player = ent
end

function ns.player.remove(ent)
    print("ns.player.remove " .. ent.id)
    if not Player.has(ent) then
        print("player not has " .. ent.id)
    end
    Player.pool[ent] = nil
end

ns.aseprite = {
    pool = {}
}

local Aseprite = ns.aseprite

function ns.aseprite.draw_all()
    for ent, e in pairs(Aseprite.pool) do
        if e.ase_data ~= nil then
            local ase_data = e.ase_data
            local dt = ns.timing.instance.dt

            local pos = ns.transform.get_world_position(ent)

            ase_data:update(dt)
            local ox = ase_data:width() / 2
            local oy = -ase_data:height()
            ase_data:draw(pos.x, pos.y, 0, 1, -1, ox, oy)
        end
    end
end

function ns.aseprite.draw_ui()

end

function ns.aseprite._flush(ent)
    print("ns.aseprite._flush " .. ent.id)

    local e = Aseprite.pool[ent]

    if e ~= nil and e["ase"] ~= nil then
        e.ase_data = neko.sprite_load(e["ase"])
    end
end

function ns.aseprite.set_ase(ent, data)
    print("ns.aseprite.set_ase " .. ent.id .. " " .. tostring(data))

    Aseprite.pool[ent]["ase"] = data

    Aseprite._flush(ent)
end

function ns.aseprite.has(ent)
    print("ns.aseprite.has " .. ent.id)
    return (Aseprite.pool[ent] ~= nil)
end

function ns.aseprite.add(ent)
    print("ns.aseprite.add " .. ent.id)

    Aseprite.pool[ent] = {
        ase = nil,
        ase_data = nil
    }

    Aseprite._flush(ent)
end

function ns.aseprite.remove(ent)
    print("ns.aseprite.remove " .. ent.id)
    if not Aseprite.has(ent) then
        print("aseprite not has " .. ent.id)
    end
    Aseprite.pool[ent] = nil
end
