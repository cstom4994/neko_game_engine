local ffi = FFI

function ns.edit.destroy_rec()
    for ent in pairs(ns.edit.select) do
        ns.transform.destroy_rec(ent)
    end

    ns.edit.undo_save()
end

function ns.edit.destroy()
    for ent in pairs(ns.edit.select) do
        ns.entity.destroy(ent)
    end

    ns.edit.undo_save()
end

function ns.edit.duplicate()
    -- save just current selection to a string
    for ent in pairs(ns.edit.select) do
        if ns.transform.has(ent) then
            ns.transform.set_save_filter_rec(ent, true)
        else
            ns.entity.set_save_filter(ent, true)
        end
    end
    local s = ng.store_open()
    ns.system.save_all(s)
    local str = ffi.string(ng.store_write_str(s))
    ng.store_close(s)

    -- clear selection
    ns.edit.select_clear()

    -- load from the string -- they were selected on save and so will be
    -- selected when loaded
    local d = ng.store_open_str(str)
    ns.system.load_all(d)
    ng.store_close(d)

    ns.edit.undo_save()
end
