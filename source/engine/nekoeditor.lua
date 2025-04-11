-- hot_require 'nekogame.edit'
local ImGui = neko.imgui

ns.edit = {
    inspect = false,
    devui_visible = false,
    mode_text = ""
}

local EditorMouseX = 0
local EditorMouseY = 0

local function EditorGetMouseUnit()
    return neko.pixels_to_unit(EditorMouseX, EditorMouseY)
end

ns.edit.set_enabled = neko.edit_set_enabled
ns.edit.get_enabled = neko.edit_get_enabled
ns.edit.set_editable = neko.edit_set_editable
ns.edit.get_editable = neko.edit_get_editable
ns.edit.set_grid_size = neko.edit_set_grid_size
ns.edit.get_grid_size = neko.edit_get_grid_size
-- ns.edit.bboxes_update = neko.edit_bboxes_update
ns.edit.bboxes_has = neko.edit_bboxes_has
ns.edit.bboxes_get_num = neko.edit_bboxes_get_num
ns.edit.bboxes_get_nth_ent = neko.edit_bboxes_get_nth_ent
ns.edit.bboxes_set_selected = neko.edit_bboxes_set_selected
ns.edit.line_add = neko.edit_line_add

local PropertyMeta = {}

-- called when using ng.add { ... }, just do nothing
function ns.edit.add()
end

function ns.edit.has()
    return true
end

--- mode management-------------------------------------------------------------

ns.edit.modes = {
    ["none"] = {},
    ["all"] = {}
}
ns.edit.mode = "none" -- 当前编辑器模式

function ns.edit.mode_event(evt)
    -- 转到到all模式 然后到当前模式
    local e = ns.edit.modes.all[evt]
    if e then
        e()
    end
    local e = ns.edit.modes[ns.edit.mode][evt]
    if e then
        e()
    end
end

function ns.edit.__ModeExecBind(up, codestr)
    local mods = { -- 修饰符前缀
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
    codestr = up and ('^' .. codestr) or codestr -- 放开按键的事件名以^开头
    ns.edit.mode_event(codestr)
end

function ns.edit.ModeKeyDown(key)
    ns.edit.__ModeExecBind(false, key)
end

function ns.edit.ModeKeyUp(key)
    ns.edit.__ModeExecBind(true, key)
end

function ns.edit.ModeMouseDown(mouse)
    ns.edit.__ModeExecBind(false, mouse)
end

function ns.edit.ModeMouseUp(mouse)
    ns.edit.__ModeExecBind(true, mouse)
end

function ns.edit.SetMode(mode)
    ns.edit.mode_event('exit')
    ns.edit.mode = mode
    ns.edit.mode_event('enter')
end

function ns.edit.set_mode_text(s)
    ns.edit.mode_text = s
end

--- play/stop/pause ------------------------------------------------------------

ns.edit.stopped = true

-- 停止时加载
local stop_savepoint = nil
local stop_save_next_frame = false -- whether to save a stop soon
local function stop_save()
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

end

function ns.edit.undo()
    if #ns.edit.history <= 1 then
        print('nothing to undo')
        return
    end

    table.remove(ns.edit.history)
    local str = ns.edit.history[#ns.edit.history]
    -- local s = ng.store_open_str(str)
    -- ns.system.load_all(s)
    -- ng.store_close(s)
end

--- normal mode ----------------------------------------------------------------

ns.edit.modes.normal = {}

function ns.edit.modes.normal.enter()

end

ns.edit.mode = 'normal' -- start in normal mode

--- edit camera ----------------------------------------------------------------

local camera_default_height = 150
ns.edit.camera = ng.add {
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
    camera_drag_mouse_prev = EditorGetMouseUnit()
    ns.edit_camera_drag.enabled = true
end

function ns.edit.camera_drag_end()
    ns.edit_camera_drag.enabled = false
end

ns.edit_camera_drag = {
    enabled = false
}
function ns.edit_camera_drag.OnUpdate()
    if not ns.edit.get_enabled() then
        ns.edit_camera_drag.enabled = false
        return
    end

    local m = EditorGetMouseUnit()

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

-- 编辑器内容
local SelectTable = ng.entity_table()
local SingleSelectID = 0
local SingleSelectEnt = nil

-- 获取一些选定的实体 如果没有则返回nil
function ns.edit.select_get_first()
    for ent in pairs(SelectTable) do
        return ent
    end
    return nil
end

function ns.edit.select_toggle(ent)
    if SelectTable[ent] then
        SelectTable[ent] = nil
    else
        SelectTable[ent] = true
    end
end

function ns.edit.select_clear()
    SelectTable = ng.entity_table()
end

--- 单击选择 ---------------------------------------------------------------

local function GetEntitiesUnderMouse()
    local ents = {}
    local mouse_pos = ns.camera.unit_to_world(EditorGetMouseUnit())
    for i = 0, ns.edit.bboxes_get_num() - 1 do
        local ent = ns.edit.bboxes_get_nth_ent(i)
        local bbox = neko.edit_bboxes_get_nth_bbox(i)
        local wmat = neko.transform_get_world_matrix(ent)
        local on = false
        if neko.bbox_contains2(bbox, wmat, mouse_pos) then
            table.insert(ents, ent)
            on = true
        end

        print("GetEntitiesUnderMouse", ent, ent.id, bbox, on)
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
    local ents = GetEntitiesUnderMouse()

    if #ents == 0 then
        SelectTable = ng.entity_table()
        ns.edit.undo_save()
        return
    end

    -- 如果已经选择了什么 则选择下一个实体
    local sel = 0
    for i = 1, #ents do
        if SelectTable[ents[i]] then
            sel = i
            break
        end
    end
    SelectTable = ng.entity_table()
    sel = (sel % #ents) + 1
    SelectTable[ents[sel]] = true

    print("上一个选择", SingleSelectEnt, SingleSelectID)
    -- ns.edit_inspector.remove(SingleSelectEnt, "transform")
    -- ns.edit_inspector.remove(SingleSelectEnt, "sprite")

    SingleSelectID = ents[sel].id
    SingleSelectEnt = ents[sel]

    print("选择", ents[sel], ents[sel].id)
    -- ns.edit_inspector.add(SingleSelectEnt, "transform")
    -- ns.edit_inspector.add(SingleSelectEnt, "sprite")

    ns.edit.undo_save()
end

function ns.edit.select_click_multi()
    -- 判断鼠标下是否有实体
    local ents = GetEntitiesUnderMouse()
    if #ents == 0 then
        ns.edit.undo_save()
        return
    end

    -- 如果没有东西被选择则选择一个实体
    for i = 1, #ents do
        if not SelectTable[ents[i]] then
            SelectTable[ents[i]] = true
            ns.edit.undo_save()
            return
        end
    end

    -- 否则取消第一个选择
    SelectTable[ents[1]] = nil

    ns.edit.undo_save()
end

--- 抓取模式 -----------------------------------------------------------------------
local grab_old_pos, grab_mouse_start
local grab_disp -- 'extra' displacement on top of mouse motion
local grab_snap -- whether snapping to grid

function ns.edit.grab_start()
    ns.edit.SetMode('grab')
end

function ns.edit.grab_end()
    ns.edit.SetMode('normal')
    ns.edit.undo_save()
end

function ns.edit.grab_cancel()
    for ent in pairs(SelectTable) do
        ns.transform.set_position(ent, grab_old_pos[ent])
    end
    ns.edit.SetMode('normal')
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
    grab_mouse_start = EditorGetMouseUnit()
    grab_disp = ng.Vec2(ng.vec2_zero)
    grab_snap = false

    -- store old positions
    grab_old_pos = ng.entity_table()
    for ent in pairs(SelectTable) do
        grab_old_pos[ent] = ns.transform.get_position(ent)
    end
end

function ns.edit.modes.grab.OnUpdate()
    local ms = ns.camera.unit_to_world(grab_mouse_start)
    local mc = ns.camera.unit_to_world(EditorGetMouseUnit())

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
    for ent in pairs(SelectTable) do
        -- move only if no ancestor is being moved (avoid double-move)
        local anc = ns.transform.get_parent(ent)
        while not ng.is_nil_entity(anc) and not SelectTable[anc] do
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
    ns.edit.SetMode('rotate')
end

function ns.edit.rotate_end()
    ns.edit.SetMode('normal')
    ns.edit.undo_save()
end

function ns.edit.rotate_cancel()
    for ent in pairs(SelectTable) do
        ns.transform.set_position(ent, rotate_old_posrot[ent].pos)
        ns.transform.set_rotation(ent, rotate_old_posrot[ent].rot)
    end
    ns.edit.SetMode('normal')
end

ns.edit.modes.rotate = {}

function ns.edit.modes.rotate.enter()
    ns.edit.set_mode_text('rotate')

    rotate_mouse_start = EditorGetMouseUnit()

    -- store old positions, rotations
    rotate_old_posrot = ng.entity_table()
    for ent in pairs(SelectTable) do
        rotate_old_posrot[ent] = {
            pos = ns.transform.get_position(ent),
            rot = ns.transform.get_rotation(ent)
        }
    end

    -- compute pivot (currently just the median)
    local n = 0
    rotate_pivot = ng.vec2_zero
    for ent in pairs(SelectTable) do
        rotate_pivot = rotate_pivot + ns.transform.get_world_position(ent)
        n = n + 1
    end
    rotate_pivot = rotate_pivot / n
end

function ns.edit.modes.rotate.OnUpdate()
    local ms = ns.camera.unit_to_world(rotate_mouse_start)
    local mc = ns.camera.unit_to_world(EditorGetMouseUnit())
    local ang = ng.vec2_atan2(mc - rotate_pivot) - ng.vec2_atan2(ms - rotate_pivot)

    for ent in pairs(SelectTable) do
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

--- edit_field -----------------------------------------------------------------

local field_types = {}

-- edit_field 使得为各种简单属性提供编辑器很容易

-- 'args' may contain:
--      field_type: the field type (boolean, string, f32, enum, Vec2,
--                                  Color, CEntity)
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
    local update_func = field_types[field.type].update
    if update_func then
        update_func(field, val)
    end
end

--- 将显示更新为 val 如果已编辑 则使用新值调用 setter
---@param field any
function ng.edit_field_post_update(field, ...)
    local post_update_func = field_types[field.type].post_update
    if post_update_func then
        post_update_func(field, unpack({...}))
    end
end

local function field_create_common(args)
    local field = {}
    return field
end

field_types["boolean"] = {
    create = function(args)
        local field = field_create_common(args)
        field.checkbox = {}
        return field
    end,

    post_update = function(field, val, setter)

    end,

    update = function(field, val)
        ImGui.Text("%s", tostring(val))
    end
}

field_types["string"] = {
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

field_types["f32"] = {
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

field_types["userdata"] = {
    create = function(args)
        local field = field_create_common(args)
        return field
    end,

    post_update = function(field, val, setter)

    end,

    update = function(field, val)
        ImGui.Text("userdata %s", tostring(val))
    end
}

-- if it's a C enum field the C enum values are automatically used, else a
-- set of values must be provided as an extra parameter to
-- ng.edit_field_post_update(...)
field_types["enum"] = {
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

field_types["Vec2"] = {
    create = function(args)
        local field = field_create_common(args)
        field.x_field = ng.edit_field_create {
            field_type = 'f32',
            ctype = 'f32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.y_field = ng.edit_field_create {
            field_type = 'f32',
            ctype = 'f32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        return field
    end,

    post_update = function(field, val, setter)
        ng.edit_field_post_update(field.x_field, val.x, function(x)
            setter(ng.Vec2(x, val.y))
        end)
        ng.edit_field_post_update(field.y_field, val.y, function(y)
            setter(ng.Vec2(val.x, y))
        end)
    end,

    update = function(field, val)
        ImGui.Text("<%f,%f>", val.x, val.y)
    end
}

field_types["vec2"] = field_types["Vec2"]

field_types["Color"] = {
    create = function(args)
        local field = field_create_common(args)
        field.r_field = ng.edit_field_create {
            field_type = 'f32',
            ctype = 'f32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.g_field = ng.edit_field_create {
            field_type = 'f32',
            ctype = 'f32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.b_field = ng.edit_field_create {
            field_type = 'f32',
            ctype = 'f32',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.a_field = ng.edit_field_create {
            field_type = 'f32',
            ctype = 'f32',
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

field_types["CEntity"] = {
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
        --     SelectTable[val] = true
        -- end
    end,

    update = function(field, val)
        ImGui.Text("%d", val.id)
    end
}

field_types["BBox"] = {
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

ns.edit_inspector.custom = {} -- 自定义监视器

local inspectors = ng.entity_table() -- CEntity (sys --> inspector) map

-- 转发事件给自定义监视器
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

    local field_type = type(r)
    local ctype = nil
    if field_type == 'cdata' or field_type == 'userdata' then
        -- 如果是userdata就尝试调用__tosting方法
        field_type = tostring(r)
    end
    if field_type == 'number' then
        field_type = 'f32'
    end

    return field_type, ctype
end

--- 添加属性字段
---@param inspector any
---@param name string
local function add_property(inspector, name)
    if inspector.props[name] then -- 已经存在
        return
    end

    local field_type, ctype = property_type(inspector, name) -- 是否为合法属性字段
    if not field_type or not field_types[field_type] then
        -- print("add_property not found", field_type, field_types[field_type])
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

--- 为检查器添加所有字段
--- @param inspector any
local function add_properties(inspector)
    if PropertyMeta[inspector.sys] then -- 从PropertyMeta查表
        for _, p in ipairs(PropertyMeta[inspector.sys]) do
            add_property(inspector, p.name)
        end
    end

    if rawget(ns, inspector.sys) then -- 通过自动发现
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
--- @param ent any 检查实体ent
--- @param sys any 检查系统的类型
local function make_inspector(ent, sys)
    local inspector = {}

    inspector.ent = ent
    inspector.sys = sys

    -- 最初位置靠近实体
    local pos = ng.Vec2(16, -16)
    if ns.transform.has(ent) then
        pos = ns.transform.local_to_world(ent, ng.vec2_zero)
    end

    inspector.last_pos = pos
    inspector.docked = false

    -- 属性字段
    inspector.props = {}
    add_properties(inspector)

    custom_event(inspector, 'add')
    return inspector
end

--- 为 CEntity ent 添加系统检查器
--- @param ent any 检查实体ent
--- @param sys any 检查系统的类型
function ns.edit_inspector.add(ent, sys)
    local adder = ns[sys].add
    if not adder then
        error("system '" .. sys .. "' has no 'add(...)' function")
    end

    if not inspectors[ent] then
        inspectors[ent] = {}
    end

    if inspectors[ent][sys] then -- 已经存在
        return
    end
    if not ns[sys].has(ent) then -- 如果此时ent实体上没有绑定sys系统
        adder(ent)
    end
    inspectors[ent][sys] = make_inspector(ent, sys) -- 创建检查器
end

--- 为CEntity移除所有系统检查器
--- @param ent any 检查实体ent
--- @param sys any 检查系统的类型 如果未指定sys则会删除ent上的所有检查器
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

--- 返回所有有效系统检查器的表
function ns.edit_inspector.get_systems()
    local sys = {}
    -- 系统必须具有属性metadata或add函数
    for k in pairs(PropertyMeta) do
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
    -- 如果关闭窗口 保存撤消点
    local some_closed = false

    for ent, insps in pairs(inspectors) do
        if ns.entity.destroyed(ent) then -- 如果实体已经被销毁
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

local function update_inspector(inspector)

    -- ns.gui_window.set_highlight(inspector.window, SelectTable[inspector.ent])
    local title = inspector.sys
    -- ns.gui_window.set_title(inspector.window, title)

    local window<close> = ImGuiTabItem(title)
    if window then
        ImGui.Text("Inspector:\n%d\n%s", inspector.ent.id, table.show(SelectTable))

        add_properties(inspector) -- 捕获新添加的属性

        ImGui.Separator()

        -- 显示属性
        for k, prop in pairs(inspector.props) do
            ImGui.Text("%s.%s", inspector.sys, prop.name)
            ng.edit_field_update(prop.field, ng.get(inspector.sys, prop.name, inspector.ent))
        end
        custom_event(inspector, 'update')
    end
end

function ns.edit_inspector.OnUpdate()
    remove_destroyed()
    if not ns.edit.get_enabled() then
        return
    end

end

local function post_update_inspector(inspector)
    -- 从inspector到目标实体绘制一条线
    if ns.transform.has(inspector.ent) then
        local a = ns.transform.local_to_world(inspector.ent, ng.Vec2(0, -16)) -- 这里应该用imgui窗口的pos
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

function ns.edit_inspector.OnPostUpdate()
    -- print("ns.edit_inspector.OnPostUpdate")

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

-- 系统特定

-- 递归销毁transform
function ns.edit.destroy_rec()
    for ent in pairs(SelectTable) do
        ns.transform.destroy_rec(ent)
    end

    ns.edit.undo_save()
end

-- 递归销毁实体
function ns.edit.destroy()
    for ent in pairs(SelectTable) do
        ns.entity.destroy(ent)
    end

    ns.edit.undo_save()
end

function ns.edit.duplicate()
    -- 仅将当前选择保存到字符串
    for ent in pairs(SelectTable) do
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

-- 默认按键绑定

-- 正常模式
ns.edit.modes.normal["KC_U"] = ns.edit.undo

ns.edit.modes.normal["KC_P"] = ns.edit.pause_toggle
ns.edit.modes.normal["S-KC_P"] = ns.edit.stop

ns.edit.modes.normal["KC_A"] = ns.edit.select_clear
ns.edit.modes.normal["MC_LEFT"] = ns.edit.select_click_single
ns.edit.modes.normal["C-MC_LEFT"] = ns.edit.select_click_multi

ns.edit.modes.normal["x"] = ns.edit.destroy
ns.edit.modes.normal["S-x"] = ns.edit.destroy_rec
ns.edit.modes.normal["S-d"] = ns.edit.duplicate

ns.edit.modes.normal["S-MC_LEFT"] = ns.edit.camera_drag_start
ns.edit.modes.normal["^S-MC_LEFT"] = ns.edit.camera_drag_end
ns.edit.modes.normal["MC_MIDDLE"] = ns.edit.camera_drag_start
ns.edit.modes.normal["^MC_MIDDLE"] = ns.edit.camera_drag_end
ns.edit.modes.normal["KC_MINUS"] = ns.edit.camera_zoom_out
ns.edit.modes.normal["KC_EQUAL"] = ns.edit.camera_zoom_in

ns.edit.modes.normal["KC_G"] = ns.edit.grab_start
ns.edit.modes.normal["KC_R"] = ns.edit.rotate_start

-- 抓取模式
ns.edit.modes.grab["KC_ENTER"] = ns.edit.grab_end
ns.edit.modes.grab["KC_ESCAPE"] = ns.edit.grab_cancel
ns.edit.modes.grab["MC_LEFT"] = ns.edit.grab_end
ns.edit.modes.grab["MC_RIGHT"] = ns.edit.grab_cancel
ns.edit.modes.grab["KC_G"] = ns.edit.grab_snap_on
ns.edit.modes.grab["KC_LEFT"] = ns.edit.grab_move_left
ns.edit.modes.grab["KC_RIGHT"] = ns.edit.grab_move_right
ns.edit.modes.grab["KC_UP"] = ns.edit.grab_move_up
ns.edit.modes.grab["KC_DOWN"] = ns.edit.grab_move_down
ns.edit.modes.grab["S-KC_LEFT"] = function()
    ns.edit.grab_move_left(5)
end
ns.edit.modes.grab["S-KC_RIGHT"] = function()
    ns.edit.grab_move_right(5)
end
ns.edit.modes.grab["S-KC_UP"] = function()
    ns.edit.grab_move_up(5)
end
ns.edit.modes.grab["S-KC_DOWN"] = function()
    ns.edit.grab_move_down(5)
end

-- 旋转模式
ns.edit.modes.rotate["KC_ENTER"] = ns.edit.rotate_end
ns.edit.modes.rotate["KC_ESCAPE"] = ns.edit.rotate_cancel
ns.edit.modes.rotate["MC_LEFT"] = ns.edit.rotate_end
ns.edit.modes.rotate["MC_RIGHT"] = ns.edit.rotate_cancel

--- 主事件 ----------------------------------------------------------------

function ns.edit.__OnKeyUp(key)
    ns.edit.ModeKeyUp(key)
end

function ns.edit.__OnKeyDown(key)
    ns.edit.ModeKeyDown(key)
end

function ns.edit.__OnMouseDown(mouse)
    ns.edit.ModeMouseDown(mouse)
end

function ns.edit.__OnMouseUp(mouse)
    ns.edit.ModeMouseUp(mouse)
end

function ns.edit.__OnMouseScroll(scroll)
    ns.edit.camera_zoom((scroll.y > 0 and 0.9 or -0.9) + 0.1 * scroll.y)
end

function ns.edit.__OnEvent(evt, args)
    if not ns.edit.get_enabled() then
        return
    end

    -- ns.edit.mode_event(evt, key, mouse)

    -- 转到系统事件
    if evt == 'OnMouseMove' then
        EditorMouseX = args.x
        EditorMouseY = -args.y -- 和input_get_mouse_pos_pixels一样需要倒置y轴
    elseif evt == 'OnKeyUp' then
        ns.edit.__OnKeyUp(args)
    elseif evt == 'OnKeyDown' then
        ns.edit.__OnKeyDown(args)
    elseif evt == 'OnMouseDown' then
        ns.edit.__OnMouseDown(args)
    elseif evt == 'OnMouseUp' then
        ns.edit.__OnMouseUp(args)
    elseif evt == 'OnMouseScroll' then
        ns.edit.__OnMouseScroll(args)
    end
end

function ns.edit.OnUpdate()
    for ent in pairs(SelectTable) do
        if ns.entity.destroyed(ent) then
            SelectTable[ent] = nil
        end
    end

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
    ns.edit.mode_event('OnUpdate')

end

function ns.edit.__OnImGui(args)

    if args == "E" then

        -- local tabbar = ImGuiTabBar("tabbar_editor")
        if ImGui.BeginTabBar("tabbar_editor") then

            if ImGui.BeginTabItem("检查") then

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

                local nselect = 0
                for w in pairs(SelectTable) do
                    nselect = nselect + 1
                    ImGui.Text(tostring(w))
                end
                if nselect > 0 then
                    ImGui.Text("SelectN: " .. nselect)
                    ImGui.Text("Select: " .. SingleSelectID .. " [" .. tostring(SingleSelectEnt) .. "]")
                else
                    ImGui.Text("no select")
                end

                -- update play/stop text
                if ns.edit.stopped then
                    ImGui.Text("Stopped")
                else
                    ImGui.Text("Running")
                end

                ImGui.EndTabItem()
            end

            if ImGui.BeginTabItem("测试面板") then
                neko.EditorTestPanelInternal()
                EditorTestPanel()
                ImGui.EndTabItem()
            end

            ImGui.EndTabBar()
        end

    elseif args == "O" then

        for _, insps in pairs(inspectors) do
            for _, inspector in pairs(insps) do
                update_inspector(inspector)
            end
        end

    end

end

function ns.edit.OnPostUpdate()
    ns.edit.mode_event("OnPostUpdate")

    -- 设置选中的bbox高光
    for i = 0, ns.edit.bboxes_get_num() - 1 do
        local ent = ns.edit.bboxes_get_nth_ent(i)
        -- local bbox = neko.edit_bboxes_get_nth_bbox(i)
        ns.edit.bboxes_set_selected(ent, SelectTable[ent] ~= nil)
    end

    if stop_save_next_frame then
        stop_save()
        stop_save_next_frame = false
    end
end

function ns.edit.save_all()
    return {
        sel = SelectTable
    }
end

function ns.edit.load_all(d)
    ng.entity_table_merge(SelectTable, d.sel)
end

--- C 系统属性 --------------------------------------------------------

PropertyMeta["transform"] = {{
    name = "parent"
}, {
    name = "position"
}, {
    name = "rotation"
}, {
    name = "scale"
}}

PropertyMeta["camera"] = {{
    name = "current"
}, {
    name = "viewport_height"
}}

PropertyMeta["sprite"] = {{
    name = "size"
}, {
    name = "texcell"
}, {
    name = "texsize"
}, {
    name = "depth"
}}
