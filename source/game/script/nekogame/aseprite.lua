ns.aseprite = {
    pool = {}
}

local Aseprite<const> = ns.aseprite

function Aseprite.OnUpdate()
    for ent, e in pairs(Aseprite.pool) do
        if e.ase_data ~= nil then
            local ase_data = e.ase_data
            local dt = neko.dt()
            ase_data:update(dt)
            ase_data:play "Idle"
        end
    end
end

function Aseprite.OnDraw()
    for ent, e in pairs(Aseprite.pool) do
        if e.ase_data ~= nil then
            local ase_data = e.ase_data
            local dt = neko.dt()

            local pos = ns.transform.get_position(ent)

            local sx = 1

            local ox = sx * ase_data:width() / 2
            local oy = -ase_data:height() / 2 - 10
            ase_data:draw(pos.x, pos.y, 0, sx * 1, -1, ox, oy)
        end
    end
end

function Aseprite.OnDrawUI()

end

function Aseprite._flush(ent)
    print("Aseprite._flush " .. ent.id)

    local e = Aseprite.pool[ent]

    if e ~= nil and e.ase ~= nil then
        e.ase_data = neko.sprite_load(e.ase)
    end
end

function Aseprite.set_ase(ent, data)
    print("Aseprite.set_ase " .. ent.id .. " " .. tostring(data))

    Aseprite.pool[ent].ase = data

    Aseprite._flush(ent)
end

function Aseprite.has(ent)
    print("Aseprite.has " .. ent.id)
    return (Aseprite.pool[ent] ~= nil)
end

function Aseprite.add(ent)
    print("Aseprite.add " .. ent.id)

    Aseprite.pool[ent] = {
        ase = nil,
        ase_data = nil
    }

    Aseprite._flush(ent)
end

function Aseprite.remove(ent)
    print("Aseprite.remove " .. ent.id)
    if not Aseprite.has(ent) then
        print("aseprite not has " .. ent.id)
    end
    Aseprite.pool[ent] = nil
end
