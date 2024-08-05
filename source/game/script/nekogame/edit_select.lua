ns.edit.select = ng.entity_table()

-- get some selected entity, or nil if none selected
function ns.edit.select_get_first()
    for ent in pairs(ns.edit.select) do
        return ent
    end
    return nil
end

function ns.edit.select_toggle(ent)
    if ns.edit.select[ent] then
        ns.edit.select[ent] = nil
    else
        ns.edit.select[ent] = true
    end
end

function ns.edit.select_clear()
    ns.edit.select = ng.entity_table()
end

--- click select ---------------------------------------------------------------

local function _get_entities_under_mouse()
    local ents = {}

    local m = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())
    for i = 0, ns.edit.bboxes_get_num() - 1 do
        local ent = ns.edit.bboxes_get_nth_ent(i)
        local bbox = ns.edit.bboxes_get_nth_bbox(i)

        -- transform m to local space
        local t = ng.mat3_inverse(ns.transform.get_world_matrix(ent))
        if ng.bbox_contains(bbox, ng.mat3_transform(t, m)) then
            table.insert(ents, ng.Entity(ent))
        end
    end

    -- sort by distance to mouse
    local distcomp = function(e1, e2)
        local p1 = ns.transform.get_world_position(e1)
        local p2 = ns.transform.get_world_position(e2)
        return ng.vec2_dist(p1, m) < ng.vec2_dist(p2, m)
    end
    table.sort(ents, distcomp)

    return ents
end

function ns.edit.select_click_single()
    -- 判断鼠标下有东西
    local ents = _get_entities_under_mouse()

    if #ents == 0 then
        ns.edit.select = ng.entity_table()
        ns.edit.undo_save()
        return
    end

    -- if something's already selected, select the next thing
    ents[#ents + 1] = ents[1] -- duplicate first at end to wrap-around
    local sel = 0
    for i = 1, #ents - 1 do
        sel = i
        if ns.edit.select[ents[i]] then
            break
        end
    end
    ns.edit.select = ng.entity_table()
    ns.edit.select[ents[sel + 1]] = true

    ns.edit.undo_save()
end

function ns.edit.select_click_multi()
    -- anything under mouse?
    local ents = _get_entities_under_mouse()
    if #ents == 0 then
        ns.edit.undo_save()
        return
    end

    -- if something isn't selected, select it
    for i = 1, #ents do
        if not ns.edit.select[ents[i]] then
            ns.edit.select[ents[i]] = true
            ns.edit.undo_save()
            return
        end
    end

    -- otherwise deselect the first
    ns.edit.select[ents[1]] = nil

    ns.edit.undo_save()
end

--- boxsel ---------------------------------------------------------------------

local boxsel_has_begun, boxsel_init_mouse_pos

local function _boxsel_select()
end

function ns.edit.boxsel_start()
    ns.edit.set_mode('boxsel')
end
function ns.edit.boxsel_begin()
    boxsel_has_begun = true
    boxsel_init_mouse_pos = ns.input.get_mouse_pos_pixels()
end
function ns.edit.boxsel_end()
    ns.edit.select = ng.entity_table()
    ns.edit.boxsel_end_add()
end
function ns.edit.boxsel_end_add()
    if boxsel_has_begun then
        b = ns.camera.pixels_to_world(boxsel_init_mouse_pos)
        e = ns.camera.pixels_to_world(ns.input.get_mouse_pos_pixels())
        bb = ng.BBox(ng.bbox_bound(b, e))

        for i = 0, ns.edit.bboxes_get_num() - 1 do
            local ent = ng.Entity(ns.edit.bboxes_get_nth_ent(i))
            if ng.bbox_contains(bb, ns.transform.get_world_position(ent)) then
                ns.edit.select[ent] = true
            end
        end
    end

    ns.edit.set_mode('normal')
    ns.edit.undo_save()
end

ns.edit.boxsel_box = ng.add {
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    transform = {
        position = ng.vec2(-20, -20)
    },
    gui_rect = {
        size = ng.vec2(10, 10),
        hfit = false,
        vfit = false
    },
    gui = {
        captures_events = false,
        color = ng.color(0.8, 0.5, 0.1, 0.3)
    }
}

ns.edit.modes.boxsel = {}

function ns.edit.modes.boxsel.enter()
    ns.edit.set_mode_text('boxsel')

    boxsel_has_begun = false
end

function ns.edit.modes.boxsel.exit()
    ns.transform.set_position(ns.edit.boxsel_box, ng.vec2(-20, -20))
    ns.gui_rect.set_size(ns.edit.boxsel_box, ng.vec2(10, 10))
end

function ns.edit.modes.boxsel.update_all()
    if not boxsel_has_begun then
        return
    end

    m = ns.input.get_mouse_pos_pixels()
    b = ng.BBox(ng.bbox_bound(m, boxsel_init_mouse_pos))
    ns.transform.set_position(ns.edit.boxsel_box, ng.vec2(b.min.x, b.max.y))
    ns.gui_rect.set_size(ns.edit.boxsel_box, b.max - b.min)
end
