--- grab -----------------------------------------------------------------------
local grab_old_pos, grab_mouse_start
local grab_disp -- 'extra' displacement on top of mouse motion
local grab_snap -- whether snapping to grid

function ns.edit.grab_start()
    ns.edit.set_mode('grab')
end
function ns.edit.grab_end()
    ns.edit.set_mode('normal')
    ns.edit.undo_save()
end
function ns.edit.grab_cancel()
    for ent in pairs(ns.edit.select) do
        ns.transform.set_position(ent, grab_old_pos[ent])
    end
    ns.edit.set_mode('normal')
end

function ns.edit.grab_snap_on()
    grab_snap = true
end
function ns.edit.grab_snap_off()
    grab_snap = false
end

-- move all selected grid size times mult in a direction
function ns.edit.grab_move_left(mult)
    local g = ns.edit.get_grid_size()
    grab_disp.x = grab_disp.x - (mult or 1) * (g.x > 0 and g.x or 1)
end
function ns.edit.grab_move_right(mult)
    local g = ns.edit.get_grid_size()
    grab_disp.x = grab_disp.x + (mult or 1) * (g.x > 0 and g.x or 1)
end
function ns.edit.grab_move_up(mult)
    local g = ns.edit.get_grid_size()
    grab_disp.y = grab_disp.y + (mult or 1) * (g.y > 0 and g.y or 1)
end
function ns.edit.grab_move_down(mult)
    local g = ns.edit.get_grid_size()
    grab_disp.y = grab_disp.y - (mult or 1) * (g.y > 0 and g.y or 1)
end

ns.edit.modes.grab = {}

function ns.edit.modes.grab.enter()
    grab_mouse_start = ns.input.get_mouse_pos_unit()
    grab_disp = ng.Vec2(ng.vec2_zero)
    grab_snap = false

    -- store old positions
    grab_old_pos = ng.entity_table()
    for ent in pairs(ns.edit.select) do
        grab_old_pos[ent] = ns.transform.get_position(ent)
    end
end

function ns.edit.modes.grab.update_all()
    local ms = ns.camera.unit_to_world(grab_mouse_start)
    local mc = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())

    -- snap mc to grid if needed
    if grab_snap then
        local g = ns.edit.get_grid_size()
        if g.x > 0 then
            mc.x = g.x * math.floor(0.5 + (mc.x - ms.x) / g.x) + ms.x
        end
        if g.y > 0 then
            mc.y = g.y * math.floor(0.5 + (mc.y - ms.y) / g.y) + ms.y
        end
    end

    -- move selected objects
    for ent in pairs(ns.edit.select) do
        -- move only if no ancestor is being moved (avoid double-move)
        local anc = ns.transform.get_parent(ent)
        while anc ~= ng.entity_nil and not ns.edit.select[anc] do
            anc = ns.transform.get_parent(anc)
        end
        if anc == ng.entity_nil then
            -- find translation in parent space
            local parent = ns.transform.get_parent(ent)
            local m = ng.mat3_inverse(ns.transform.get_world_matrix(parent))
            local d = ng.mat3_transform(m, mc) - ng.mat3_transform(m, ms)
            d = d + ng.mat3_transform(m, grab_disp) - ng.mat3_transform(m, ng.vec2_zero)
            ns.transform.set_position(ent, grab_old_pos[ent] + d)
        end
    end

    -- update
    local snap_text = grab_snap and 'snap ' or ''
    local d = mc - ms + grab_disp
    local mode_text = string.format('grab %s%.4f, %.4f', snap_text, d.x, d.y)
    ns.edit.set_mode_text(mode_text)
end

--- rotate ---------------------------------------------------------------------

local rotate_old_posrot, rotate_mouse_start, rotate_pivot

function ns.edit.rotate_start()
    ns.edit.set_mode('rotate')
end
function ns.edit.rotate_end()
    ns.edit.set_mode('normal')
    ns.edit.undo_save()
end
function ns.edit.rotate_cancel()
    for ent in pairs(ns.edit.select) do
        ns.transform.set_position(ent, rotate_old_posrot[ent].pos)
        ns.transform.set_rotation(ent, rotate_old_posrot[ent].rot)
    end
    ns.edit.set_mode('normal')
end

ns.edit.modes.rotate = {}

function ns.edit.modes.rotate.enter()
    ns.edit.set_mode_text('rotate')

    rotate_mouse_start = ns.input.get_mouse_pos_unit()

    -- store old positions, rotations
    rotate_old_posrot = ng.entity_table()
    for ent in pairs(ns.edit.select) do
        rotate_old_posrot[ent] = {
            pos = ns.transform.get_position(ent),
            rot = ns.transform.get_rotation(ent)
        }
    end

    -- compute pivot (currently just the median)
    local n = 0
    rotate_pivot = ng.vec2_zero
    for ent in pairs(ns.edit.select) do
        rotate_pivot = rotate_pivot + ns.transform.get_world_position(ent)
        n = n + 1
    end
    rotate_pivot = rotate_pivot / n
end

function ns.edit.modes.rotate.update_all()
    local ms = ns.camera.unit_to_world(rotate_mouse_start)
    local mc = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())
    local ang = ng.vec2_atan2(mc - rotate_pivot) - ng.vec2_atan2(ms - rotate_pivot)

    for ent in pairs(ns.edit.select) do
        -- set new rotation
        ns.transform.set_rotation(ent, rotate_old_posrot[ent].rot + ang)

        -- compute new position
        local parent = ns.transform.get_parent(ent)
        local m = ns.transform.get_world_matrix(parent)
        local wpos = ng.mat3_transform(m, rotate_old_posrot[ent].pos)
        local d = wpos - rotate_pivot
        d = ng.vec2_rot(d, ang)
        wpos = rotate_pivot + d

        -- set new position
        local im = ng.mat3_inverse(m)
        ns.transform.set_position(ent, ng.mat3_transform(im, wpos))
    end
end
