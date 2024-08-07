local ffi = FFI

ns.edit = {
    inspect = false
}

--- expose C functions ---------------------------------------------------------

ns.edit.set_enabled = ng.edit_set_enabled
ns.edit.get_enabled = ng.edit_get_enabled

ns.edit.set_editable = ng.edit_set_editable
ns.edit.get_editable = ng.edit_get_editable

ns.edit.set_grid_size = ng.edit_set_grid_size
ns.edit.get_grid_size = ng.edit_get_grid_size

ns.edit.bboxes_update = ng.edit_bboxes_update
ns.edit.bboxes_has = ng.edit_bboxes_has
ns.edit.bboxes_get = ng.edit_bboxes_get
ns.edit.bboxes_get_num = ng.edit_bboxes_get_num
ns.edit.bboxes_get_nth_ent = ng.edit_bboxes_get_nth_ent
ns.edit.bboxes_get_nth_bbox = ng.edit_bboxes_get_nth_bbox
ns.edit.bboxes_set_selected = ng.edit_bboxes_set_selected

ns.edit.line_add = ng.edit_line_add

-- called when using ng.add { ... }, just do nothing
function ns.edit.add()
end

function ns.edit.has()
    return true
end

--- mode management-------------------------------------------------------------

ns.edit.modes = {
    ['none'] = {},
    ['all'] = {}
}
ns.edit.mode = 'none'

function ns.edit.mode_event(evt)
    -- convert key/mouse enum to number
    if type(evt) == 'cdata' then
        evt = tonumber(evt)
    end

    -- forward to all, then to current mode
    local e = ns.edit.modes.all[evt]
    if e then
        e()
    end
    local e = ns.edit.modes[ns.edit.mode][evt]
    if e then
        e()
    end
end

-- codestr is 'a', 'b', 'c', '<tab>' etc. as returned by input.*_to_string(...)
function ns.edit._mode_exec_bind(up, codestr)
    -- modifier prefixes
    local mods = {
        [ng.KC_LEFT_CONTROL] = 'C-',
        [ng.KC_RIGHT_CONTROL] = 'C-',
        [ng.KC_LEFT_ALT] = 'M-',
        [ng.KC_RIGHT_ALT] = 'M-',
        [ng.KC_LEFT_SHIFT] = 'S-',
        [ng.KC_RIGHT_SHIFT] = 'S-'
    }
    for mod, prefix in pairs(mods) do
        if ns.input.key_down(mod) then
            codestr = prefix .. codestr
        end
    end

    -- up prefix
    codestr = up and ('^' .. codestr) or codestr

    -- execute!
    ns.edit.mode_event(codestr)
end

function ns.edit.mode_key_down(key)
    if ns.gui.captured_event() then
        return
    end
    ns.edit._mode_exec_bind(false, ns.input.keycode_to_string(key))
end
function ns.edit.mode_key_up(key)
    if ns.gui.captured_event() then
        return
    end
    ns.edit._mode_exec_bind(true, ns.input.keycode_to_string(key))
end
function ns.edit.mode_mouse_down(mouse)
    if ns.gui.captured_event() then
        return
    end
    ns.edit._mode_exec_bind(false, ns.input.mousecode_to_string(mouse))
end
function ns.edit.mode_mouse_up(mouse)
    if ns.gui.captured_event() then
        return
    end
    ns.edit._mode_exec_bind(true, ns.input.mousecode_to_string(mouse))
end

function ns.edit.set_mode(mode)
    ns.edit.mode_event('exit')
    ns.edit.mode = mode
    ns.edit.mode_event('enter')
end

--- play/stop/pause ------------------------------------------------------------

ns.edit.stopped = true

-- load this when stopped
local stop_savepoint = nil
local stop_save_next_frame = false -- whether to save a stop soon
local function stop_save()
    ns.group.set_save_filter('default edit_inspector', true)
    local s = ng.store_open()
    ns.system.save_all(s)
    stop_savepoint = ffi.string(ng.store_write_str(s))
    ng.store_close(s)

    if ns.timing.get_paused() then
        ns.edit.stopped = true
    end
end
function ns.edit.stop_save()
    stop_save_next_frame = true
end

function ns.edit.stop()
    if not stop_savepoint then
        return
    end

    ns.group.destroy('default edit_inspector')
    local s = ng.store_open_str(stop_savepoint)
    ns.system.load_all(s)
    ng.store_close(s)

    ns.timing.set_paused(true)
    ns.edit.stopped = true
end

function ns.edit.play()
    ns.timing.set_paused(false)
end

function ns.edit.pause_toggle()
    ns.timing.set_paused(not ns.timing.get_paused())
end

--- undo -----------------------------------------------------------------------

ns.edit.history = {}

function ns.edit.undo_save()
    ns.group.set_save_filter('default edit_inspector', true)
    local s = ng.store_open()
    ns.system.save_all(s)

    local str = ffi.string(ng.store_write_str(s))
    table.insert(ns.edit.history, str)
    if ns.edit.stopped then
        stop_savepoint = str
    end -- update stop if stopped

    ng.store_close(s)
end

function ns.edit.undo()
    if #ns.edit.history <= 1 then
        print('nothing to undo')
        return
    end

    -- TODO: make 'edit' entity group and destroy all except that?
    ns.group.destroy('default edit_inspector')

    table.remove(ns.edit.history)
    local str = ns.edit.history[#ns.edit.history]
    local s = ng.store_open_str(str)
    ns.system.load_all(s)
    ng.store_close(s)
end

--- normal mode ----------------------------------------------------------------

ns.edit.modes.normal = {}

function ns.edit.modes.normal.enter()
    ns.edit.hide_mode_text()
end

ns.edit.mode = 'normal' -- start in normal mode

--- edit gui root --------------------------------------------------------------

ns.edit.gui_root = ng.add {
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true,
        vfill = true
    },
    gui = {
        visible = false,
        captures_events = false,
        color = ng.color(0, 0, 0, 0), -- invisible
        halign = ng.GA_MIN,
        valign = ng.GA_MIN,
        padding = ng.vec2_zero
    }
}

--- edit camera ----------------------------------------------------------------

local camera_default_height = 25
ns.edit.camera = ng.add {
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    camera = {
        viewport_height = camera_default_height,
        current = false
    }
}
ns.camera.set_edit_camera(ns.edit.camera)

-- drag

local camera_drag_mouse_prev
function ns.edit.camera_drag_start()
    camera_drag_mouse_prev = ns.input.get_mouse_pos_unit()
    ns.edit_camera_drag.enabled = true
end
function ns.edit.camera_drag_end()
    ns.edit_camera_drag.enabled = false
end

ns.edit_camera_drag = {
    enabled = false
}
function ns.edit_camera_drag.update_all()
    if not ns.edit.get_enabled() then
        ns.edit_camera_drag.enabled = false
        return
    end

    local m = ns.input.get_mouse_pos_unit()

    -- find screen mouse motion in world coordinates, move opposite way
    local campos = ns.transform.local_to_world(ns.edit.camera, ng.vec2_zero)
    local mp = ns.camera.unit_to_world(camera_drag_mouse_prev) - campos
    local mc = ns.camera.unit_to_world(m) - campos
    ns.transform.translate(ns.edit.camera, mp - mc)
    camera_drag_mouse_prev = m
end

-- zoom
local camera_zoom_factor = 0
function ns.edit.camera_zoom(f)
    camera_zoom_factor = camera_zoom_factor + f
    local h = math.pow(0.8, camera_zoom_factor) * camera_default_height
    ns.camera.set_viewport_height(ns.edit.camera, h)
end
function ns.edit.camera_zoom_in()
    ns.edit.camera_zoom(1)
end
function ns.edit.camera_zoom_out()
    ns.edit.camera_zoom(-1)
end

--- other files ----------------------------------------------------------------

-- core edit stuff
hot_require 'nekogame.edit_select'
hot_require 'nekogame.edit_command'
hot_require 'nekogame.edit_bottom_gui'
hot_require 'nekogame.edit_inspector'

-- system-specific
hot_require 'nekogame.edit_entity'
hot_require 'nekogame.edit_transform'
hot_require 'nekogame.edit_physics'

-- default binds
hot_require 'nekogame.edit_binds'

--- main events ----------------------------------------------------------------

function ns.edit.key_up(key)
    if not ns.edit.get_enabled() then
        return
    end
    ns.edit.mode_key_up(key)
end
function ns.edit.key_down(key)
    if not ns.edit.get_enabled() then
        return
    end
    ns.edit.mode_key_down(key)
end
function ns.edit.mouse_down(mouse)
    if not ns.edit.get_enabled() then
        return
    end
    ns.edit.mode_mouse_down(mouse)
end
function ns.edit.mouse_up(mouse)
    if not ns.edit.get_enabled() then
        return
    end
    ns.edit.mode_mouse_up(mouse)
end
function ns.edit.scroll(scroll)
    ns.edit.camera_zoom((scroll.y > 0 and 0.9 or -0.9) + 0.1 * scroll.y)
end

function ns.edit.update_all()
    for ent in pairs(ns.edit.select) do
        if ns.entity.destroyed(ent) then
            ns.edit.select[ent] = nil
        end
    end

    if ns.gui.event_mouse_down(ns.edit.play_text) == ng.MC_LEFT then
        if ns.edit.stopped then
            ns.edit.play()
        else
            ns.edit.stop()
        end
    end
    if not ns.timing.get_paused() then
        ns.edit.stopped = false
    end

    -- if not enabled skip -- also handle gui visibility
    if not ns.edit.get_enabled() then
        ns.gui.set_visible(ns.edit.gui_root, false)
        return
    end
    ns.gui.set_visible(ns.edit.gui_root, true)

    -- forward to mode
    ns.edit.mode_event('update_all')

    -- update grid text
    local g = ns.edit.get_grid_size()
    if g.x <= 0 and g.y <= 0 then
        ns.gui.set_visible(ns.edit.grid_textbox, false)
    else
        ns.gui.set_visible(ns.edit.grid_textbox, true)
        local s
        if g.x == g.y then
            s = string.format('grid %.4g', g.x)
        else
            s = string.format('grid %.4g %.4g', g.x, g.y)
        end
        ns.gui_text.set_str(ns.edit.grid_text, s)
    end

    -- update select text
    local nselect = 0
    for _ in pairs(ns.edit.select) do
        nselect = nselect + 1
    end
    if nselect > 0 then
        ns.gui.set_visible(ns.edit.select_textbox, true)
        ns.gui_text.set_str(ns.edit.select_text, 'select ' .. nselect)
    else
        ns.gui.set_visible(ns.edit.select_textbox, false)
        ns.gui_text.set_str(ns.edit.select_text, '')
    end

    -- update play/stop text
    if ns.edit.stopped then
        ns.gui_text.set_str(ns.edit.play_text, '\x10')
    else
        ns.gui_text.set_str(ns.edit.play_text, '\xcb')
    end
end

function ns.edit.post_update_all()

    -- print("ns.edit.post_update_all")

    ns.edit.mode_event('post_update_all')

    -- update bbox highlight
    for i = 0, ns.edit.bboxes_get_num() - 1 do
        local ent = ns.edit.bboxes_get_nth_ent(i)
        local bbox = ns.edit.bboxes_get_nth_bbox(i)
        ns.edit.bboxes_set_selected(ent, ns.edit.select[ent] ~= nil)
    end

    -- save stop?
    if stop_save_next_frame then
        stop_save()
        stop_save_next_frame = false
    end
end

function ns.edit.save_all()
    return {
        sel = ns.edit.select
    }
end
function ns.edit.load_all(d)
    ng.entity_table_merge(ns.edit.select, d.sel)
end
