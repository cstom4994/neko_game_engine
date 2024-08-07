local ffi = FFI
local refct = hot_require 'reflect'

--- edit_field -----------------------------------------------------------------

local field_types = {}

-- edit_field makes it easy to provide editors for simple properties of various
-- types (Vec2, Scalar etc.)

-- 'args' may contain:
--      field_type: the field type (boolean, string, Scalar, enum, Vec2,
--                                  Color, Entity)
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

-- updates display to val
function ng.edit_field_update(field, val)
    local f = field_types[field.type].update
    if f then
        f(field, val)
    end
end

-- updates display to val, calls setter with new value if edited
function ng.edit_field_post_update(field, ...)
    local f = field_types[field.type].post_update
    if f then
        f(field, unpack({...}))
    end
end

-- returns textbox, text
local function field_create_textbox(args)
    local textbox = ng.add {
        transform = {
            parent = args.field.container
        },
        gui = {
            color = ng.color(0.2, 0.2, 0.4, 1),
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        },
        gui_textbox = {}
    }

    local text = ns.gui_textbox.get_text(textbox)
    if args.editable == nil or args.editable or args.numerical then
        ns.gui_textedit.add(text)
        ns.gui_textbox.set_click_focus(textbox, true)
    end
    if args.numerical then
        ns.gui_textedit.set_numerical(text, true)
    end

    return textbox, text
end

local function field_create_common(args)
    local field = {}

    field.container = ng.add {
        transform = {
            parent = args.parent
        },
        gui = {
            padding = ng.vec2_zero,
            color = ng.color_clear,
            valign = args.valign or ng.GA_TABLE,
            halign = args.halign or ng.GA_MIN
        },
        gui_rect = {
            hfill = true
        }
    }

    if args.label then
        field.name = args.label
        field.label = ng.add {
            transform = {
                parent = field.container
            },
            gui = {
                color = ng.color_white,
                valign = ng.GA_MID,
                halign = ng.GA_TABLE
            },
            gui_text = {
                str = args.label
            }
        }
    end

    return field
end

field_types['boolean'] = {
    create = function(args)
        local field = field_create_common(args)
        field.checkbox = ng.add {
            transform = {
                parent = field.container
            },
            gui = {
                color = ng.color(0.2, 0.2, 0.4, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_checkbox = {}
        }
        return field
    end,

    post_update = function(field, val, setter)
        if ns.gui.event_changed(field.checkbox) then
            setter(ns.gui_checkbox.get_checked(field.checkbox))
            ns.edit.undo_save()
        else
            ns.gui_checkbox.set_checked(field.checkbox, val)
        end
    end
}

field_types['string'] = {
    create = function(args)
        local field = field_create_common(args)
        field.textbox, field.textedit = field_create_textbox {
            field = field
        }
        return field
    end,

    post_update = function(field, val, setter)
        if ns.gui.event_focus_exit(field.textedit) then
            ns.edit.undo_save()
        end
        if ns.gui.event_changed(field.textedit) then
            setter(ns.gui_text.get_str(field.textedit))
        elseif not ns.gui.get_focus(field.textedit) then
            ns.gui_text.set_str(field.textedit, val)
        end
    end
}

field_types['Scalar'] = {
    create = function(args)
        local field = field_create_common(args)
        field.textbox, field.textedit = field_create_textbox {
            field = field,
            numerical = true
        }
        return field
    end,

    post_update = function(field, val, setter)
        if ns.gui.event_focus_exit(field.textedit) then
            ns.edit.undo_save()
        end
        if ns.gui.event_changed(field.textedit) then
            setter(ns.gui_textedit.get_num(field.textedit))
        elseif not ns.gui.get_focus(field.textedit) then
            ns.gui_text.set_str(field.textedit, string.format('%.4f', val))
        end
    end
}

-- if it's a C enum field the C enum values are automatically used, else a
-- set of values must be provided as an extra parameter to
-- ng.edit_field_post_update(...)
field_types['enum'] = {
    create = function(args)
        local field = field_create_common(args)
        field.enumtype = args.ctype
        field.textbox, field.text = field_create_textbox {
            field = field,
            editable = false
        }
        return field
    end,

    post_update = function(field, val, setter, vals)
        if ns.gui.event_mouse_down(field.textbox) == ng.MC_LEFT then
            local function setter_wrap(s)
                setter(s)
                ns.edit.undo_save()
            end
            vals = field.enumtype and ng.enum_values(field.enumtype) or vals
            local comp = ns.edit.command_completion_substr(vals)
            local prompt = 'set ' .. (field.name or '') .. ': '
            ns.edit.command_start(prompt, setter_wrap, comp, true)
        end
        val = field.enumtype and ng.enum_tostring(field.enumtype, val) or val
        ns.gui_text.set_str(field.text, val)
    end
}

field_types['Vec2'] = {
    create = function(args)
        local field = field_create_common(args)
        field.x_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.y_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
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
    end
}

field_types['Color'] = {
    create = function(args)
        local field = field_create_common(args)
        field.r_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.g_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.b_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
            parent = field.container,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        }
        field.a_field = ng.edit_field_create {
            field_type = 'Scalar',
            ctype = 'Scalar',
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

field_types['Entity'] = {
    create = function(args)
        local field = field_create_common(args)
        field.enumtype = args.ctype
        field.textbox, field.text = field_create_textbox {
            field = field,
            editable = false
        }

        -- 'set' button
        field.pick = ng.add {
            transform = {
                parent = field.container
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(field.pick), 'set')

        return field
    end,

    post_update = function(field, val, setter)
        -- pick new value?
        if ns.gui.event_mouse_down(field.pick) == ng.MC_LEFT then
            val = ns.edit.select_get_first() or ng.entity_nil
            setter(val)
            ns.edit.undo_save()
        end

        -- display, select on click
        ns.gui_text.set_str(field.text, val == ng.entity_nil and '(nil)' or string.format('[%d]', val.id))
        if ns.gui.event_mouse_down(field.textbox) == ng.MC_LEFT and val ~= ng.entity_nil then
            ns.edit.select_clear()
            ns.edit.select[val] = true
        end
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

local inspectors = ng.entity_table() -- Entity (sys --> inspector) map

ns.edit_inspector.gui_root = ng.add {
    transform = {
        parent = ns.edit.gui_root
    },
    group = {
        groups = 'builtin edit_inspector'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true,
        vfill = true
    },
    gui = {
        captures_events = false,
        color = ng.color(0, 0, 0, 0), -- invisible
        halign = ng.GA_MIN,
        valign = ng.GA_MAX,
        padding = ng.vec2_zero
    }
}

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

-- returns field type, C type
local function property_type(inspector, name)
    local r = ng.get(inspector.sys, name, inspector.ent)

    local field_type = type(r)
    local ctype = nil
    if field_type == 'cdata' then
        local refctt = refct.typeof(r)
        ctype = refctt.name
        if refctt.what == 'enum' then
            field_type = 'enum'
        else
            field_type = ctype
        end
    end
    if field_type == 'number' then
        field_type = 'Scalar'
    end

    return field_type, ctype
end

-- add field for property
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
            parent = inspector.window_body,
            label = name
        }
    }
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

-- return inspector object
local function make_inspector(ent, sys)
    local inspector = {}

    inspector.ent = ng.Entity(ent)
    inspector.sys = sys

    -- put near entity initially
    local pos = ng.vec2(16, -16)
    if ns.transform.has(ent) then
        pos = ns.transform.local_to_world(ent, ng.vec2_zero)
        pos = ns.transform.world_to_local(ns.edit_inspector.gui_root, pos) + ng.vec2(22, -10)
    end

    -- window
    inspector.window = ng.add {
        transform = {
            parent = ns.edit_inspector.gui_root,
            position = pos
        },
        gui_window = {},
        gui = {}
    }
    inspector.last_pos = pos
    inspector.window_body = ns.gui_window.get_body(inspector.window)
    inspector.docked = false

    -- dock toggle button
    inspector.dock_text = ng.add {
        transform = {
            parent = ns.gui_window.get_title_buttons_area(inspector.window)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        },
        gui_text = {
            str = '<'
        }
    }

    -- 'remove' button
    inspector.remove_text = ng.add {
        transform = {
            parent = ns.gui_window.get_title_buttons_area(inspector.window)
        },
        gui = {
            color = ng.color_red,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE
        },
        gui_text = {
            str = 'r'
        }
    }

    -- property fields
    inspector.props = {}
    add_properties(inspector)

    custom_event(inspector, 'add')
    return inspector
end

-- add a sys inspector for Entity ent
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

-- remove sys inspector for Entity ent -- sys is optional, removes all
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
    ns.gui_window.remove(inspectors[ent][sys].window)
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
                if ns.gui.event_mouse_down(inspector.remove_text) == ng.MC_LEFT then
                    ns[inspector.sys].remove(inspector.ent)
                    ns.edit_inspector.remove(inspector.ent, inspector.sys)
                    some_closed = true
                elseif ns.entity.destroyed(inspector.window) or not ns[inspector.sys].has(ent) then
                    ns.edit_inspector.remove(inspector.ent, inspector.sys)
                    some_closed = true
                end
            end
        end
    end

    if some_closed then
        ns.edit.undo_save()
    end
end

-- make entities uneditable/unsaveable etc. recursively
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
    ns.transform.set_parent(inspector.window, ns.edit_inspector.gui_root)

    ns.gui_window.set_highlight(inspector.window, ns.edit.select[inspector.ent])
    local title = inspector.sys
    ns.gui_window.set_title(inspector.window, title)

    -- docking
    if ns.gui.event_mouse_down(inspector.dock_text) == ng.MC_LEFT then
        inspector.docked = not inspector.docked
        if not inspector.docked then
            ns.transform.set_position(inspector.window, inspector.last_pos)
        end
    end
    if inspector.docked then
        ns.gui.set_halign(inspector.window, ng.GA_MAX)
        ns.gui.set_valign(inspector.window, ng.GA_TABLE)
        ns.gui_text.set_str(inspector.dock_text, '<')
    else
        inspector.last_pos = ns.transform.get_position(inspector.window)
        ns.gui.set_halign(inspector.window, ng.GA_NONE)
        ns.gui.set_valign(inspector.window, ng.GA_NONE)
        ns.gui_text.set_str(inspector.dock_text, '>')
    end

    add_properties(inspector) -- capture newly added properties

    update_group_editable_rec(inspector.window)

    for _, prop in pairs(inspector.props) do
        ng.edit_field_update(prop.field, ng.get(inspector.sys, prop.name, inspector.ent))
    end
    custom_event(inspector, 'update')
end

function ns.edit_inspector.update_all()
    ns.transform.set_parent(ns.edit_inspector.gui_root, ns.edit.gui_root)
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
    -- draw line from inspector to target entity
    if ns.transform.has(inspector.ent) then
        local a = ns.transform.local_to_world(inspector.window, ng.vec2(0, -16))
        local b = ns.transform.local_to_world(inspector.ent, ng.vec2_zero)
        ns.edit.line_add(a, b, 0, ng.color(1, 0, 1, 0.6))
    end

    for _, prop in pairs(inspector.props) do
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

    if ns.entity.get_save_filter(ns.edit_inspector.gui_root) then
        data.gui_root = ns.edit_inspector.gui_root
    end

    data.tbl = ng.entity_table()
    for _, insps in pairs(inspectors) do
        for _, inspector in pairs(insps) do
            if ns.entity.get_save_filter(inspector.ent) then
                data.tbl[inspector.window] = inspector
            end
        end
    end

    return data
end
function ns.edit_inspector.load_all(data)
    if data.gui_root then
        ns.edit_inspector.gui_root = data.gui_root
    end

    for win, inspector in pairs(data.tbl) do
        if not inspectors[inspector.ent] then
            inspectors[inspector.ent] = {}
        end
        inspectors[inspector.ent][inspector.sys] = inspector
    end
end

--- C system properties --------------------------------------------------------

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
