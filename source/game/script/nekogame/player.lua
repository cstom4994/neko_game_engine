ns.player = {
    pool = {}
}

local Player = ns.player

function ns.player.draw_all()

end

function ns.player.draw_ui()

end

function ns.player.has(ent)
    print("ns.player.has " .. ent.id)
    return (Player.pool[ent.id] ~= nil)
end

function ns.player.add(ent)
    print("ns.player.add " .. ent.id)
end

function ns.player.remove(ent)
    print("ns.player.remove " .. ent.id)
    if not Player.has(ent) then
        print("player not has " .. ent.id)
    end
    Player.pool[ent.id] = nil
end
