-- hot_require 'nekogame.edit'
local ffi = FFI

local ImGui = neko.imgui

ns.edit = {
    inspect = false,
    devui_visible = false,
    command_text = "",
    command_text_colon = "",
    mode_text = ""
}

--- expose C functions ---------------------------------------------------------

ns.edit.set_enabled = neko.edit_set_enabled
ns.edit.get_enabled = neko.edit_get_enabled

ns.edit.set_editable = ng.edit_set_editable
ns.edit.get_editable = ng.edit_get_editable

ns.edit.set_grid_size = ng.edit_set_grid_size
ns.edit.get_grid_size = ng.edit_get_grid_size

-- ns.edit.bboxes_update = ng.edit_bboxes_update
ns.edit.bboxes_has = ng.edit_bboxes_has
ns.edit.bboxes_get_num = ng.edit_bboxes_get_num
ns.edit.bboxes_get_nth_ent = ng.edit_bboxes_get_nth_ent
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
        ["lctrl"] = 'C-',
        ["rctrl"] = 'C-',
        ["lalt"] = 'M-',
        ["ralt"] = 'M-',
        ["lshift"] = 'S-',
        ["rshift"] = 'S-'
    }
    for mod, prefix in pairs(mods) do
        if neko.key_down(mod) then
            codestr = prefix .. codestr
        end
    end

    -- up prefix
    codestr = up and ('^' .. codestr) or codestr

    -- execute!
    ns.edit.mode_event(codestr)
end

function ns.edit.mode_key_down(key)
    if neko.gui_captured_event() then
        return
    end
    ns.edit._mode_exec_bind(false, ns.input.keycode_to_string(key))
end

function ns.edit.mode_key_up(key)
    if neko.gui_captured_event() then
        return
    end
    ns.edit._mode_exec_bind(true, ns.input.keycode_to_string(key))
end

function ns.edit.mode_mouse_down(mouse)
    if neko.gui_captured_event() then
        return
    end
    ns.edit._mode_exec_bind(false, ns.input.mousecode_to_string(mouse))
end

function ns.edit.mode_mouse_up(mouse)
    if neko.gui_captured_event() then
        return
    end
    ns.edit._mode_exec_bind(true, ns.input.mousecode_to_string(mouse))
end

function ns.edit.set_mode(mode)
    ns.edit.mode_event('exit')
    ns.edit.mode = mode
    ns.edit.mode_event('enter')
end

function ns.edit.set_mode_text(s)
    ns.edit.mode_text = s
end

--- play/stop/pause ------------------------------------------------------------

ns.edit.stopped = true

-- load this when stopped
local stop_savepoint = nil
local stop_save_next_frame = false -- whether to save a stop soon
local function stop_save()
    ns.group.set_save_filter('default edit_inspector', true)
    -- local s = ng.store_open()
    -- ns.system.save_all(s)
    -- stop_savepoint = ffi.string(ng.store_write_str(s))
    -- ng.store_close(s)

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
    -- local s = ng.store_open_str(stop_savepoint)
    -- ns.system.load_all(s)
    -- ng.store_close(s)

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
    -- ns.group.set_save_filter('default edit_inspector', true)
    -- local s = ng.store_open()
    -- ns.system.save_all(s)

    -- local str = ffi.string(ng.store_write_str(s))
    -- table.insert(ns.edit.history, str)
    -- if ns.edit.stopped then
    --     stop_savepoint = str
    -- end -- update stop if stopped

    -- ng.store_close(s)
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
    -- local s = ng.store_open_str(str)
    -- ns.system.load_all(s)
    -- ng.store_close(s)
end

--- normal mode ----------------------------------------------------------------

ns.edit.modes.normal = {}

function ns.edit.modes.normal.enter()
    ns.edit.hide_mode_text()
end

ns.edit.mode = 'normal' -- start in normal mode

--- edit camera ----------------------------------------------------------------

local camera_default_height = 150
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
-- hot_require 'nekogame.edit_select'
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

--- 单击选择 ---------------------------------------------------------------

local function _get_entities_under_mouse()

    local ents = {}

    local mouse_pos = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())
    for i = 0, ns.edit.bboxes_get_num() - 1 do
        local ent = ns.edit.bboxes_get_nth_ent(i)
        local bbox = neko.edit_bboxes_get_nth_bbox(i)
        local wmat = neko.transform_get_world_matrix(ent.id)
        if neko.bbox_contains2(bbox, wmat, mouse_pos) then
            table.insert(ents, ng.NativeEntity(ent))
        end

        print("_get_entities_under_mouse", ent.id, bbox)
    end

    -- 按与鼠标的距离排序
    local distcomp = function(e1, e2)
        local p1 = ns.transform.get_world_position(e1)
        local p2 = ns.transform.get_world_position(e2)
        return ng.vec2_dist(p1, mouse_pos) < ng.vec2_dist(p2, mouse_pos)
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

-- hot_require 'nekogame.edit_command'
--- command mode ---------------------------------------------------------------
ns.edit.modes.command = {}

local command_end_callback, command_completion_func, command_completions
local command_completions_index, command_always_complete

local function command_update_completions_text()
    ns.edit.command_completions_text = table.concat(command_completions, ' | ')
end
local function command_update_completions()
    local s = ns.edit.command_text
    command_completions = command_completion_func(s)
    command_update_completions_text()
end

-- whether sub is a subsequence of seq
local function subseq(seq, sub)
    local j = 1
    local lsub = #sub + 1
    if lsub == 1 then
        return true
    end
    for i = 1, #seq do
        if string.byte(seq, i) == string.byte(sub, j) then
            j = j + 1
            if j == lsub then
                return true
            end
        end
    end
    return false
end

-- returns a completion function that uses substring search
-- case insensitive
function ns.edit.command_completion_substr(t)
    return function(s)
        local comps = {}
        local s = string.lower(s)
        for k in pairs(t) do
            if subseq(string.lower(k), s) then
                table.insert(comps, k)
            end
        end
        return comps
    end
end

-- 使用文件系统搜索的补全函数 不区分大小写
function ns.edit.command_completion_fs(s)
    local comps = {}
    -- local s = string.lower(s)
    -- local dir_path = string.match(s, '(.*/)') or './'
    -- local suffix = string.match(s, '.*/(.*)') or s
    -- local dir = ns.fs.dir_open(dir_path)
    -- if dir == nil then
    --     return {}
    -- end
    -- while true do
    --     local f = ns.fs.dir_next_file(dir)
    --     if f == nil then
    --         break
    --     end
    --     f = ng.string(f)
    --     if f ~= '.' and f ~= '..' and subseq(string.lower(dir_path .. f), s) then
    --         table.insert(comps, dir_path .. f)
    --     end
    -- end
    -- ns.fs.dir_close(dir)
    return comps
end

local function run_string(s)
    local r, e = loadstring(s)
    if r then
        r()
    else
        error(e)
    end
end

function ns.edit.command_start(prompt, callback, completion_func, always_complete, initial)
    ns.edit.set_mode('command')

    -- default is eval script
    prompt = prompt or 'lua: '
    command_end_callback = callback or run_string
    command_completion_func = completion_func or function()
        return {}
    end
    command_always_complete = always_complete and true or false

    initial = initial or ''
    ns.edit.command_text = initial or ''
    -- ns.gui_textedit.set_cursor(ns.edit.command_text, #initial)

    ns.edit.command_text_colon = prompt
    command_update_completions()
end

function ns.edit.command_end()
    if command_always_complete then
        if #command_completions == 0 then
            return
        end -- no completions
        ns.edit.command_complete()
    end

    ns.edit.set_mode('normal')

    local s = ns.edit.command_text
    if command_end_callback then
        command_end_callback(s)
    else
        print('no command callback for \'' .. s .. '\'')
    end
end

function ns.edit.command_cancel()
    ns.edit.set_mode('normal')
end

-- actually pick a completion
function ns.edit.command_complete()
    if #command_completions > 0 then
        local comp = command_completions[1]
        ns.edit.command_text = comp
        -- ns.gui_textedit.set_cursor(ns.edit.command_text, #comp)
        command_update_completions()
    end
end

function ns.edit.modes.command.enter()
    ns.edit.set_mode_text('command')

    ns.edit.command_completions_text = ''
end

function ns.edit.modes.command.exit()
    command_completions = {}
end

function ns.edit.modes.command.update_all()
    -- if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_ENTER then
    --     ns.edit.command_end()
    --     return
    -- elseif ns.gui.event_focus_exit(ns.edit.command_text) then
    --     ns.edit.command_cancel()
    --     return
    -- end

    -- if ns.gui.event_changed(ns.edit.command_text) then
    --     command_update_completions()
    -- end
    -- if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_TAB then
    --     ns.edit.command_complete()
    -- end

    -- -- next/prev completion
    -- if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_DOWN then
    --     table.insert(command_completions, table.remove(command_completions, 1))
    --     command_update_completions_text()
    -- elseif ns.gui.event_key_down(ns.edit.command_text) == ng.KC_UP then
    --     table.insert(command_completions, 1, table.remove(command_completions))
    --     command_update_completions_text()
    -- end
end

--- 内置提示 -----------------------------------------------------------

-- 询问网格大小
local function command_gridx(x)
    local function gridy(y)
        ns.edit.set_grid_size(ng.vec2(tonumber(x) or 0, tonumber(y) or 0))
    end
    ns.edit.command_start('grid y: ', gridy)
end
function ns.edit.command_grid()
    ns.edit.command_start('grid x: ', command_gridx)
end

-- 检查所选实体的系统 如果未选择实体 则创建实体
function ns.edit.command_inspect()
    local add = ng.entity_table_empty(ns.edit.select)

    local function system(s)
        if add then
            local e = ng.entity_create()
            ns.edit_inspector.add(e, s)
            ns.edit.select[e] = true
        elseif not ng.entity_table_empty(ns.edit.select) then
            for ent in pairs(ns.edit.select) do
                ns.edit_inspector.add(ent, s)
            end
        end
        ns.edit.undo_save()
    end

    -- 完成已列出属性的系统
    local syss = ns.edit_inspector.get_systems()
    local comp = ns.edit.command_completion_substr(syss)

    ns.edit.command_start(add and 'new entity: ' or 'edit system: ', system, comp, true)
end

local last_save = nekogame_usr_path .. 'levels/'
function ns.edit.command_save()
    local function save(f)
        ns.console.printf("edit: saving group 'default' to file '" .. f .. "' ... ")
        ns.group.set_save_filter('default', true)
        -- local s = ng.store_open()
        -- ns.system.save_all(s)
        -- ng.store_write_file(s, f)
        -- ng.store_close(s)
        print("done")

        ns.edit.stop_save()

        last_save = f
    end

    ns.edit.command_start('save to file: ', save, ns.edit.command_completion_fs, false, last_save)
end

local last_load = nekogame_usr_path .. 'levels/'
function ns.edit.command_load()
    local function load(f)
        ns.group.destroy('default')

        ns.console.printf("edit: loading from file '" .. f .. "' ... ")
        local s = ng.store_open_file(f)
        ns.system.load_all(s)
        ng.store_close(s)
        print("done")

        ns.edit.stop_save()
        ns.timing.set_paused(true)
        ns.edit.stopped = true

        last_load = f
    end

    ns.edit.command_start('load from file: ', load, ns.edit.command_completion_fs, false, last_load)
end

function ns.edit.set_default_file(s)
    last_save = s
    last_load = s
end

local last_save_prefab = nekogame_usr_path .. 'prefabs/'
function ns.edit.command_save_prefab()
    if ng.entity_table_empty(ns.edit.select) then
        return
    end

    local function save(f)
        for ent in pairs(ns.edit.select) do
            if ns.transform.has(ent) then
                ns.transform.set_save_filter_rec(ent, true)
            else
                ns.entity.set_save_filter(ent, true)
            end
        end

        ns.prefab.save(f, ns.edit.select_get_first())

        last_save_prefab = f
    end

    ns.edit.command_start('save prefab: ', save, ns.edit.command_completion_fs, false, last_save_prefab)
end

local last_load_prefab = nekogame_usr_path .. 'prefabs/'
function ns.edit.command_load_prefab()
    local function load(f)
        ns.edit.select_clear()
        local ent = ns.prefab.load(f)
        ns.edit.select[ent] = true
        if ns.transform.has(ent) then
            -- move to center of view
            local w = ns.transform.local_to_world(ns.edit.camera, ng.vec2_zero)
            w.x = math.floor(w.x + 0.5)
            w.y = math.floor(w.y + 0.5)
            ns.transform.set_position(ent, w)
        end
        ns.edit.undo_save()

        last_load_prefab = f
    end

    ns.edit.command_start('load prefab: ', load, ns.edit.command_completion_fs, true, last_load_prefab)
end

function ns.edit.set_default_prefab_file(s)
    last_save_prefab = s
    last_load_prefab = s
end

-- hot_require 'nekogame.edit_transform'
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
        while not ng.is_nil_entity(anc) and not ns.edit.select[anc] do
            anc = ns.transform.get_parent(anc)
        end
        if ng.is_nil_entity(anc) then
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

-- hot_require 'nekogame.edit_inspector'

--- edit_field -----------------------------------------------------------------

local field_types = {}

-- edit_field makes it easy to provide editors for simple properties of various
-- types (Vec2, Float32 etc.)

-- 'args' may contain:
--      field_type: the field type (boolean, string, Float32, enum, Vec2,
--                                  Color, NativeEntity)
--      ctype: the name of the underlying C type, if any
--      parent: the parent GUI entity
--      label: descriptive label (optional)
--      halign, valign: alignment for container (optional)
function ng.edit_field_create(args)
    assert(args.field_type, 'field type expected')
    local field = field_types[args.field_type].create(args)
    field.type = args.field_type
    return field
end

--- 将显示更新为 val
---@param field any
---@param val any
function ng.edit_field_update(field, val)
    local f = field_types[field.type].update
    if f then
        f(field, val)
    end
end

--- 将显示更新为 val 如果已编辑 则使用新值调用 setter
---@param field any
function ng.edit_field_post_update(field, ...)
    local f = field_types[field.type].post_update
    if f then
        f(field, unpack({...}))
    end
end

local function field_create_common(args)
    local field = {}
    return field
end

field_types['boolean'] = {
    create = function(args)
        local field = field_create_common(args)
        field.checkbox = {}
        return field
    end,

    post_update = function(field, val, setter)

    end,

    update = function(field, val)
        ImGui.Text("%d", val)
    end
}

field_types['string'] = {
    create = function(args)
        local field = field_create_common(args)
        field.textbox = {}
        field.textedit = {}
        return field
    end,

    post_update = function(field, val, setter)

    end,

    update = function(field, val)
        ImGui.Text("%s", val)
    end
}

field_types['Float32'] = {
    create = function(args)
        local field = field_create_common(args)
        field.textbox = {}
        field.textedit = {}
        return field
    end,

    post_update = function(field, val, setter)

    end,

    update = function(field, val)
        ImGui.Text("%f", val)
    end
}

-- if it's a C enum field the C enum values are automatically used, else a
-- set of values must be provided as an extra parameter to
-- ng.edit_field_post_update(...)
field_types['enum'] = {
    create = function(args)
        local field = field_create_common(args)
        field.enumtype = args.ctype
        field.textbox = {}
        field.textedit = {}
        return field
    end,

    post_update = function(field, val, setter, vals)
        -- if ns.gui.event_mouse_down(field.textbox) == ng.MC_LEFT then
        --     local function setter_wrap(s)
        --         setter(s)
        --         ns.edit.undo_save()
        --     end
        --     vals = field.enumtype and ng.enum_values(field.enumtype) or vals
        --     local comp = ns.edit.command_completion_substr(vals)
        --     local prompt = 'set ' .. (field.name or '') .. ': '
        --     ns.edit.command_start(prompt, setter_wrap, comp, true)
        -- end
        -- val = field.enumtype and ng.enum_tostring(field.enumtype, val) or val
        -- ns.gui_text.set_str(field.text, val)
    end
}

field_types['Vec2'] = {
    create = function(args)
        local field = field_create_common(args)
        field.x_field = ng.edit_field_create {
            field_type = 'Float32',
            ctype = 'Float32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.y_field = ng.edit_field_create {
            field_type = 'Float32',
            ctype = 'Float32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        return field
    end,

    post_update = function(field, val, setter)
        ng.edit_field_post_update(field.x_field, val.x, function(x)
            setter(ng.vec2(x, val.y))
        end)
        ng.edit_field_post_update(field.y_field, val.y, function(y)
            setter(ng.vec2(val.x, y))
        end)
    end,

    update = function(field, val)
        ImGui.Text("<%f,%f>", val.x, val.y)
    end
}

field_types['Color'] = {
    create = function(args)
        local field = field_create_common(args)
        field.r_field = ng.edit_field_create {
            field_type = 'Float32',
            ctype = 'Float32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.g_field = ng.edit_field_create {
            field_type = 'Float32',
            ctype = 'Float32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.b_field = ng.edit_field_create {
            field_type = 'Float32',
            ctype = 'Float32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.a_field = ng.edit_field_create {
            field_type = 'Float32',
            ctype = 'Float32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        return field
    end,

    post_update = function(field, val, setter)
        ng.edit_field_post_update(field.r_field, val.r, function(r)
            setter(ng.color(r, val.g, val.b, val.a))
        end)
        ng.edit_field_post_update(field.g_field, val.g, function(g)
            setter(ng.color(val.r, g, val.b, val.a))
        end)
        ng.edit_field_post_update(field.b_field, val.b, function(b)
            setter(ng.color(val.r, val.g, b, val.a))
        end)
        ng.edit_field_post_update(field.a_field, val.a, function(a)
            setter(ng.color(val.r, val.g, val.b, a))
        end)
    end
}

field_types['NativeEntity'] = {
    create = function(args)
        local field = field_create_common(args)
        field.enumtype = args.ctype
        field.textbox = {}
        field.textedit = {}
        return field
    end,

    post_update = function(field, val, setter)
        -- -- pick new value?
        -- if ns.gui.event_mouse_down(field.pick) == ng.MC_LEFT then
        --     val = ns.edit.select_get_first() or ng.entity_nil
        --     setter(val)
        --     ns.edit.undo_save()
        -- end

        -- -- display, select on click
        -- ns.gui_text.set_str(field.text, ng.is_nil_entity(val) and '(nil)' or string.format('[%d]', val.id))
        -- if ns.gui.event_mouse_down(field.textbox) == ng.MC_LEFT and not ng.is_nil_entity(val) then
        --     ns.edit.select_clear()
        --     ns.edit.select[val] = true
        -- end
    end,

    update = function(field, val)
        ImGui.Text("%llu", val.id)
    end
}

field_types['BBox'] = {
    create = function(args)
        local field = field_create_common(args)
        field.min_field = ng.edit_field_create {
            field_type = 'Vec2',
            ctype = 'Vec2',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.max_field = ng.edit_field_create {
            field_type = 'Vec2',
            ctype = 'Vec2',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        return field
    end,

    post_update = function(field, val, setter)
        ng.edit_field_post_update(field.min_field, val.min, function(m)
            setter(ng.bbox(m, val.max))
        end)
        ng.edit_field_post_update(field.max_field, val.max, function(m)
            setter(ng.bbox(val.min, m))
        end)
    end
}

--- inspector ------------------------------------------------------------------

ns.edit_inspector = {
    inspect = false
}

ns.edit_inspector.custom = {} -- custom inspectors -- eg. for physics

local inspectors = ng.entity_table() -- NativeEntity (sys --> inspector) map

-- forward event to custom event handler
local function custom_event(inspector, evt)
    local custom = ns.edit_inspector.custom[inspector.sys]
    if custom then
        local f = ns.edit_inspector.custom[inspector.sys][evt]
        if f then
            f(inspector)
        end
    end
end

--- 返回字段类型和C类型
---@param inspector any
---@param name any
local function property_type(inspector, name)
    local r = ng.get(inspector.sys, name, inspector.ent)

    -- TODO 这里需要等到全面改用NekoLuaWrapper及相应反射才能
    --      通过userdata确定具体C类型
    local field_type = type(r)

    -- print("property_type", name, field_type)

    local ctype = nil
    if field_type == 'cdata' or field_type == 'userdata' then
        -- local refctt = neko.refct.typeof(r)

        -- print("ffi.typeof", ffi.typeof(r))

        -- ctype = refctt.name
        -- if refctt.what == 'enum' then
        --     field_type = 'enum'
        -- else
        --     field_type = ctype
        -- end
    end
    if field_type == 'number' then
        field_type = 'Float32'
    end

    return field_type, ctype
end

--- 添加属性字段
---@param inspector any
---@param name any
local function add_property(inspector, name)
    if inspector.props[name] then
        return
    end -- already exists

    local field_type, ctype = property_type(inspector, name)
    if not field_type or not field_types[field_type] then
        return
    end

    inspector.props[name] = {
        name = name,
        field = ng.edit_field_create {
            field_type = field_type,
            ctype = ctype,
            -- parent = inspector.window_body,
            label = name
        }
    }

    print("add_property", name)

end

-- add all properties for an inspector, either through ns.meta.props or
-- through automatic discovery
local function add_properties(inspector)
    if ns.meta.props[inspector.sys] then
        for _, p in ipairs(ns.meta.props[inspector.sys]) do
            add_property(inspector, p.name)
        end
    elseif rawget(ns, inspector.sys) then
        for f in pairs(ns[inspector.sys]) do
            if string.sub(f, 1, 4) == 'set_' then
                local prop = string.sub(f, 5, string.len(f))
                if ns[inspector.sys]['get_' .. prop] then
                    add_property(inspector, prop)
                end
            end
        end
    end
end

--- 返回检查器对象
---@param ent 检查实体ent
---@param sys 检查系统的类型
local function make_inspector(ent, sys)
    local inspector = {}

    inspector.ent = ng.NativeEntity(ent)
    inspector.sys = sys

    -- put near entity initially
    local pos = ng.vec2(16, -16)
    if ns.transform.has(ent) then
        pos = ns.transform.local_to_world(ent, ng.vec2_zero)
    end

    inspector.last_pos = pos
    inspector.docked = false

    -- property fields
    inspector.props = {}
    add_properties(inspector)

    custom_event(inspector, 'add')
    return inspector
end

-- 为 NativeEntity ent 添加系统检查器
function ns.edit_inspector.add(ent, sys)
    local adder = ns[sys].add
    if not adder then
        error("system '" .. sys .. "' has no 'add(...)' function")
    end

    if not inspectors[ent] then
        inspectors[ent] = {}
    end

    if inspectors[ent][sys] then
        return
    end
    if not ns[sys].has(ent) then
        adder(ent)
    end
    inspectors[ent][sys] = make_inspector(ent, sys)
end

-- remove sys inspector for NativeEntity ent -- sys is optional, removes all
-- inspectors on ent if not specified
function ns.edit_inspector.remove(ent, sys)
    if not inspectors[ent] then
        return
    end

    if not sys then
        for sys in pairs(inspectors[ent]) do
            ns.edit_inspector.remove(ent, sys)
        end
        inspectors[ent] = nil
        return
    end

    custom_event(inspectors[ent][sys], 'remove')
    inspectors[ent][sys] = nil
end

-- return set of all valid inspector systems
function ns.edit_inspector.get_systems()
    local sys = {}
    -- system must either have property metadata or an 'add(...)' function
    for k in pairs(ns.meta.props) do
        sys[k] = true
    end
    for k in pairs(ns) do
        if ns[k].inspect ~= false and ns[k].add then
            sys[k] = true
        end
    end
    return sys
end

local function remove_destroyed()
    -- if closed a window, save an undo point
    local some_closed = false

    for ent, insps in pairs(inspectors) do
        if ns.entity.destroyed(ent) then
            ns.edit_inspector.remove(ent)
        else
            for _, inspector in pairs(insps) do
                -- if ns.gui.event_mouse_down(inspector.remove_text) == ng.MC_LEFT then
                --     ns[inspector.sys].remove(inspector.ent)
                --     ns.edit_inspector.remove(inspector.ent, inspector.sys)
                --     some_closed = true
                -- elseif ns.entity.destroyed(inspector.window) or not ns[inspector.sys].has(ent) then
                --     ns.edit_inspector.remove(inspector.ent, inspector.sys)
                --     some_closed = true
                -- end
            end
        end
    end

    if some_closed then
        ns.edit.undo_save()
    end
end

--- 递归地使实体不可编辑/不可保存等
---@param ent any
local function update_group_editable_rec(ent)
    ns.edit.set_editable(ent, false)
    ns.group.set_groups(ent, 'builtin edit_inspector')

    if ns.transform.has(ent) then
        local children = ns.transform.get_children(ent)
        for i = 0, ns.transform.get_num_children(ent) - 1 do
            update_group_editable_rec(children[i])
        end
    end
end

local function update_inspector(inspector)

    -- ns.gui_window.set_highlight(inspector.window, ns.edit.select[inspector.ent])
    local title = inspector.sys
    -- ns.gui_window.set_title(inspector.window, title)

    ImGui.Begin(title)

    ImGui.Text(tostring(ns.edit.select[inspector.ent]))

    add_properties(inspector) -- 捕获新添加的属性

    -- update_group_editable_rec(inspector.window)

    ImGui.Separator()

    for k, prop in pairs(inspector.props) do
        ImGui.Text("%s %s %s", k, inspector.sys, prop.name)
        ng.edit_field_update(prop.field, ng.get(inspector.sys, prop.name, inspector.ent))
    end
    custom_event(inspector, 'update')

    ImGui.End()
end

function ns.edit_inspector.update_all()
    remove_destroyed()
    if not ns.edit.get_enabled() then
        return
    end

    for _, insps in pairs(inspectors) do
        for _, inspector in pairs(insps) do
            update_inspector(inspector)
        end
    end
end

local function post_update_inspector(inspector)
    -- 从inspector到目标实体绘制一条线
    if ns.transform.has(inspector.ent) then
        local a = ns.transform.local_to_world(inspector.ent, ng.vec2(0, -16)) -- 这里应该用imgui窗口的pos
        local b = ns.transform.local_to_world(inspector.ent, ng.vec2_zero)
        ns.edit.line_add(a, b, 0, ng.color(1, 0, 1, 0.6))
    end

    for k, prop in pairs(inspector.props) do
        ng.edit_field_post_update(prop.field, ng.get(inspector.sys, prop.name, inspector.ent), function(val)
            ng.set(inspector.sys, prop.name, inspector.ent, val)
        end)
    end
    custom_event(inspector, 'post_update')
end

function ns.edit_inspector.post_update_all()
    -- print("ns.edit_inspector.post_update_all")

    remove_destroyed()
    if not ns.edit.get_enabled() then
        return
    end

    for _, insps in pairs(inspectors) do
        for _, inspector in pairs(insps) do
            post_update_inspector(inspector)
        end
    end
end

function ns.edit_inspector.save_all()
    local data = {}

    -- data.tbl = ng.entity_table()
    -- for _, insps in pairs(inspectors) do
    --     for _, inspector in pairs(insps) do
    --         if ns.entity.get_save_filter(inspector.ent) then
    --             data.tbl[inspector.window] = inspector
    --         end
    --     end
    -- end

    return data
end

function ns.edit_inspector.load_all(data)

    -- for win, inspector in pairs(data.tbl) do
    --     if not inspectors[inspector.ent] then
    --         inspectors[inspector.ent] = {}
    --     end
    --     inspectors[inspector.ent][inspector.sys] = inspector
    -- end
end

-- system-specific
-- hot_require 'nekogame.edit_entity'
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
    -- local s = ng.store_open()
    -- ns.system.save_all(s)
    -- local str = ffi.string(ng.store_write_str(s))
    -- ng.store_close(s)

    -- clear selection
    ns.edit.select_clear()

    -- load from the string -- they were selected on save and so will be
    -- selected when loaded
    -- local d = ng.store_open_str(str)
    -- ns.system.load_all(d)
    -- ng.store_close(d)

    ns.edit.undo_save()
end

-- default binds
-- hot_require 'nekogame.edit_binds'
-- normal mode
ns.edit.modes.normal['S-;'] = ns.edit.command_start
ns.edit.modes.normal['u'] = ns.edit.undo

ns.edit.modes.normal['s'] = ns.edit.command_save
ns.edit.modes.normal['l'] = ns.edit.command_load
ns.edit.modes.normal['\''] = ns.edit.command_save_prefab
ns.edit.modes.normal['.'] = ns.edit.command_load_prefab

ns.edit.modes.normal['p'] = ns.edit.pause_toggle
ns.edit.modes.normal['S-p'] = ns.edit.stop

ns.edit.modes.normal['a'] = ns.edit.select_clear
ns.edit.modes.normal['<mouse_1>'] = ns.edit.select_click_single
ns.edit.modes.normal['C-<mouse_1>'] = ns.edit.select_click_multi

ns.edit.modes.normal['x'] = ns.edit.destroy
ns.edit.modes.normal['S-x'] = ns.edit.destroy_rec
ns.edit.modes.normal['S-d'] = ns.edit.duplicate

ns.edit.modes.normal['S-<mouse_1>'] = ns.edit.camera_drag_start
ns.edit.modes.normal['^S-<mouse_1>'] = ns.edit.camera_drag_end
ns.edit.modes.normal['<mouse_3>'] = ns.edit.camera_drag_start
ns.edit.modes.normal['^<mouse_3>'] = ns.edit.camera_drag_end
ns.edit.modes.normal['-'] = ns.edit.camera_zoom_out
ns.edit.modes.normal['='] = ns.edit.camera_zoom_in

ns.edit.modes.normal['g'] = ns.edit.grab_start
ns.edit.modes.normal['r'] = ns.edit.rotate_start
ns.edit.modes.normal['b'] = ns.edit.boxsel_start

ns.edit.modes.normal[','] = ns.edit.command_inspect
ns.edit.modes.normal['S-g'] = ns.edit.command_grid

-- grab mode
ns.edit.modes.grab['<enter>'] = ns.edit.grab_end
ns.edit.modes.grab['<escape>'] = ns.edit.grab_cancel
ns.edit.modes.grab['<mouse_1>'] = ns.edit.grab_end
ns.edit.modes.grab['<mouse_2>'] = ns.edit.grab_cancel
ns.edit.modes.grab['g'] = ns.edit.grab_snap_on
ns.edit.modes.grab['<left>'] = ns.edit.grab_move_left
ns.edit.modes.grab['<right>'] = ns.edit.grab_move_right
ns.edit.modes.grab['<up>'] = ns.edit.grab_move_up
ns.edit.modes.grab['<down>'] = ns.edit.grab_move_down
ns.edit.modes.grab['S-<left>'] = function()
    ns.edit.grab_move_left(5)
end
ns.edit.modes.grab['S-<right>'] = function()
    ns.edit.grab_move_right(5)
end
ns.edit.modes.grab['S-<up>'] = function()
    ns.edit.grab_move_up(5)
end
ns.edit.modes.grab['S-<down>'] = function()
    ns.edit.grab_move_down(5)
end

-- rotate mode
ns.edit.modes.rotate['<enter>'] = ns.edit.rotate_end
ns.edit.modes.rotate['<escape>'] = ns.edit.rotate_cancel
ns.edit.modes.rotate['<mouse_1>'] = ns.edit.rotate_end
ns.edit.modes.rotate['<mouse_2>'] = ns.edit.rotate_cancel

-- -- boxsel mode
-- ns.edit.modes.boxsel['<mouse_1>'] = ns.edit.boxsel_begin
-- ns.edit.modes.boxsel['C-<mouse_1>'] = ns.edit.boxsel_begin
-- ns.edit.modes.boxsel['^<mouse_1>'] = ns.edit.boxsel_end
-- ns.edit.modes.boxsel['^C-<mouse_1>'] = ns.edit.boxsel_end_add

-- -- phypoly mode
-- ns.edit.modes.phypoly['<enter>'] = ns.edit.phypoly_end
-- ns.edit.modes.phypoly['<escape>'] = ns.edit.phypoly_cancel
-- ns.edit.modes.phypoly['<mouse_1>'] = ns.edit.phypoly_add_vertex

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

    -- if ns.edit.event_mouse_down(ns.edit.play_text) == ng.MC_LEFT then
    --     if ns.edit.stopped then
    --         ns.edit.play()
    --     else
    --         ns.edit.stop()
    --     end
    -- end
    if not ns.timing.get_paused() then
        ns.edit.stopped = false
    end

    -- if not enabled skip -- also handle gui visibility
    if not ns.edit.get_enabled() then
        ns.edit.devui_visible = false
        return
    end
    ns.edit.devui_visible = true

    -- forward to mode
    ns.edit.mode_event('update_all')

    ImGui.Begin("Editor")
    ImGui.Text("TestDemo:" .. ns.edit.mode_text)

    -- -- update grid text
    local g = ns.edit.get_grid_size()
    if g.x <= 0 and g.y <= 0 then
        ImGui.Text("no grid")
    else
        local s
        if g.x == g.y then
            s = string.format('grid %.4g', g.x)
        else
            s = string.format('grid %.4g %.4g', g.x, g.y)
        end
        ImGui.Text(s)
    end

    -- update select text
    local nselect = 0
    for _ in pairs(ns.edit.select) do
        nselect = nselect + 1
    end
    if nselect > 0 then
        ImGui.Text('select ' .. nselect)
    else
        ImGui.Text("no select")
    end

    -- update play/stop text
    if ns.edit.stopped then
        ImGui.Text('\x10')
    else
        ImGui.Text('\xcb')
    end

    ImGui.End()

end

function ns.edit.post_update_all()
    -- print("ns.edit.post_update_all")

    ns.edit.mode_event('post_update_all')

    -- update bbox highlight
    for i = 0, ns.edit.bboxes_get_num() - 1 do
        local ent = ns.edit.bboxes_get_nth_ent(i)
        -- local bbox = neko.edit_bboxes_get_nth_bbox(i)
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

--- C 系统属性 --------------------------------------------------------

ns.meta.props['transform'] = {{
    name = 'parent'
}, {
    name = 'position'
}, {
    name = 'rotation'
}, {
    name = 'scale'
}}

ns.meta.props['camera'] = {{
    name = 'current'
}, {
    name = 'viewport_height'
}}

ns.meta.props['sprite'] = {{
    name = 'size'
}, {
    name = 'texcell'
}, {
    name = 'texsize'
}, {
    name = 'depth'
}}

ns.meta.props['physics'] = {{
    name = 'type'
}, {
    name = 'mass'
}, {
    name = 'freeze_rotation'
}, {
    name = 'velocity'
}, {
    name = 'force'
}, {
    name = 'angular_velocity'
}, {
    name = 'torque'
}, {
    name = 'velocity_limit'
}, {
    name = 'angular_velocity_limit'
}}

ns.meta.props['gui'] = {{
    name = 'color'
}, {
    name = 'visible'
}, {
    name = 'focusable'
}, {
    name = 'captures_events'
}, {
    name = 'halign'
}, {
    name = 'valign'
}, {
    name = 'padding'
}}
ns.meta.props['gui_rect'] = {{
    name = 'size'
}, {
    name = 'hfit'
}, {
    name = 'vfit'
}, {
    name = 'hfill'
}, {
    name = 'vfill'
}}
ns.meta.props['gui_text'] = {{
    name = 'str'
}}
ns.meta.props['gui_textedit'] = {{
    name = 'cursor'
}, {
    name = 'numerical'
}}

ns.meta.props['sound'] = {{
    name = 'path'
}, {
    name = 'playing'
}, {
    name = 'seek'
}, {
    name = 'finish_destroy'
}, {
    name = 'loop'
}, {
    name = 'gain'
}}
