ns.aseprite = {
    pool = {}
}

local Aseprite = ns.aseprite

function ns.aseprite.update_all()
    for ent, e in pairs(Aseprite.pool) do
        if e.ase_data ~= nil then
            local ase_data = e.ase_data
            local dt = ng.get_timing_instance().dt
            ase_data:update(dt)
            ase_data:play "Idle"
        end
    end
end

function ns.aseprite.draw_all()
    for ent, e in pairs(Aseprite.pool) do
        if e.ase_data ~= nil then
            local ase_data = e.ase_data
            local dt = ng.get_timing_instance().dt

            local pos = ns.transform.get_position(ent)

            local sx = 1

            local ox = sx * ase_data:width() / 2
            local oy = -ase_data:height() / 2 - 10
            ase_data:draw(pos.x, pos.y, 0, sx * 1, -1, ox, oy)
        end
    end
end

function ns.aseprite.draw_ui()

end

function ns.aseprite._flush(ent)
    print("ns.aseprite._flush " .. ent.id)

    local e = Aseprite.pool[ent]

    if e ~= nil and e.ase ~= nil then
        e.ase_data = neko.sprite_load(e.ase)
    end
end

function ns.aseprite.set_ase(ent, data)
    print("ns.aseprite.set_ase " .. ent.id .. " " .. tostring(data))

    Aseprite.pool[ent].ase = data

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
