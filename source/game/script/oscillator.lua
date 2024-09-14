--     oscillator_set(ent, { amp = ..., freq = ... })
ns.oscillator = {
    auto_saveload = true
}

ns.oscillator.tbl = ng.entity_table()

function ns.oscillator.add(ent, osc)
    -- default parameters
    if not osc.phase then
        osc.phase = 0
    end
    if not osc.amp then
        osc.amp = 1
    end
    if not osc.freq then
        osc.freq = 1
    end

    -- initial state
    osc.initx = ng.transform_get_position(ent).x
    osc.t = 0

    ns.oscillator.tbl[ent] = osc
end

function ns.oscillator.update_all()
    for ent, _ in pairs(ns.oscillator.tbl) do
        if ns.entity.destroyed(ent) then
            ns.oscillator.tbl[ent] = nil
        end
    end

    for ent, osc in pairs(ns.oscillator.tbl) do
        local pos = ns.transform.get_position(ent)
        pos.x = osc.initx + osc.amp * math.sin(2 * math.pi * (osc.phase + osc.freq * osc.t))
        ns.transform.set_position(ent, pos)
        osc.t = osc.t + ng.get_timing_instance().dt
    end
end
