local root = ns.gui.get_root()
ns.group.set_groups(root, 'builtin')

ng.wrap_string('gui_text', 'str')


--- event ----------------------------------------------------------------------

ns.gui_event = {}

local event_handlers = ng.entity_table()
local event_defaults = {}

function ns.gui_event.add()
end

local function add_event(event, default)
    event_defaults[event] = default

    ns.gui_event['set_' .. event] = function (ent, f)
        if not event_handlers[ent] then
            event_handlers[ent] = {}
        end
        event_handlers[ent][event] = f
    end
end

add_event('focus_enter', false)
add_event('focus_exit', false)
add_event('mouse_down', ng.MC_NONE)
add_event('mouse_up', ng.MC_NONE)
add_event('key_down', ng.KC_NONE)
add_event('key_up', ng.KC_NONE)

function ns.gui_event.update_all()
    for ent in pairs(event_handlers) do
        if ns.entity.destroyed(ent) then event_handlers[ent] = nil end
    end

    for ent, handlers in pairs(event_handlers) do
        for event, f in pairs(handlers) do
            local r = ns.gui['event_' .. event](ent)
            if r ~= event_defaults[event] then
                f(ent, r)
            end
        end
    end
end


--- window ---------------------------------------------------------------------

ns.gui_window = { auto_saveload = true }

ns.gui_window.tbl = ng.entity_table()

function ns.gui_window.add(ent)
    if ns.gui_window.tbl[ent] then return end
    ns.gui_window.tbl[ent] = {}
    local window = ns.gui_window.tbl[ent]

    -- add ent to gui_rect as container
    ng.add {
        ent = ent,
        gui_rect = {},
        gui = { color = ng.color(0.3, 0.3, 0.5, 0.95) },
    }

    -- titlebar containing text, minimize button
    window.titlebar = ng.add {
        transform = { parent = ent },
        gui_rect = { hfill = true },
        gui = {
            padding = ng.vec2_zero,
            color = ng.color(0.15, 0.15, 0.35, 0.95),
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN,
        },
    }
    window.title_buttons_area = ng.add {
        transform = { parent = window.titlebar },
        gui_rect = {},
        gui = {
            padding = ng.vec2_zero,
            color = ng.color(0.0, 0.0, 0.0, 0.0),
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE,
        },
    }
    window.close_text = ng.add {
        transform = { parent = window.title_buttons_area },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE,
        },
        gui_text = { str = 'x' },
    }
    window.minmax_text = ng.add {
        transform = { parent = window.title_buttons_area },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE,
        },
        gui_text = { str = '-' },
    }
    window.title_text_area = ng.add {
        transform = { parent = window.titlebar },
        gui_rect = { hfill = true },
        gui = {
            padding = ng.vec2_zero,
            color = ng.color(0.0, 0.0, 0.0, 0.0),
            valign = ng.GA_MAX,
            halign = ng.GA_TABLE,
        },
    }
    window.title_text = ng.add {
        transform = { parent = window.title_text_area },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_MID,
        },
        gui_text = { str = 'new window' },
    }

    -- body containing contents
    window.body = ng.add {
        transform = { parent = ent },
        gui_rect = {},
        gui = {
            padding = ng.vec2_zero,
            color = ng.color(0.0, 0.0, 0.0, 0.0),
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        },
    }
end

function ns.gui_window.remove(ent)
    local window = ns.gui_window.tbl[ent]
    if window then ns.transform.destroy_rec(ent) end
    ns.gui_window.tbl[ent] = nil
end

function ns.gui_window.has(ent)
    return ns.gui_window.tbl[ent] ~= nil
end

ng.simple_props(ns.gui_window, {
    minimized = false,
    closeable = true,
    highlight = false
})

function ns.gui_window.set_title(ent, str)
    local window = ns.gui_window.tbl[ent]
    if window then ns.gui_text.set_str(window.title_text, str) end
end
function ns.gui_window.get_title(ent)
    local window = ns.gui_window.tbl[ent]
    if window then return ns.gui_text.get_str(window.title_text) end
end
function ns.gui_window.get_title_buttons_area(ent)
    local window = ns.gui_window.tbl[ent]
    if window then return window.title_buttons_area end
end

function ns.gui_window.get_body(ent)
    local window = ns.gui_window.tbl[ent]
    if window then return window.body end
end

-- window that is being dragged
local drag_window
local mouse_prev = nil, mouse_curr

function ns.gui_window.mouse_up(mouse)
    if mouse == ng.MC_LEFT then
        drag_window = nil
    end
end

function ns.gui_window.update_all()
    -- get mouse position
    local mouse_curr = ns.input.get_mouse_pos_pixels()
    if not mouse_prev then mouse_prev = mouse_curr end

    -- close button clicked?
    for ent, window in pairs(ns.gui_window.tbl) do
        if ns.gui.event_mouse_down(window.close_text) == ng.MC_LEFT
        and window.closeable then
            ns.entity.destroy(ent)
        end
    end

    -- clear destroyed
    if drag_window and ns.entity.destroyed(drag_window) then
        drag_window = nil
    end
    for ent in pairs(ns.gui_window.tbl) do
        if ns.entity.destroyed(ent) then ns.gui_window.remove(ent) end
    end

    -- update all
    for ent, window in pairs(ns.gui_window.tbl) do
        -- new drag motion?
        if ns.gui.event_mouse_down(window.titlebar) == ng.MC_LEFT
        and ns.gui.get_halign(ent) == ng.GA_NONE
        and ns.gui.get_valign(ent) == ng.GA_NONE then
            drag_window = ent
        end

        -- highlight?
        if window.highlight then
            ns.gui.set_color(window.title_text, ng.color(1, 1, 0.2, 1))
        else
            ns.gui.set_color(window.title_text, ng.color_white)
        end

        -- closeable?
        ns.gui.set_visible(window.close_text, window.closeable)

        -- update maximize/minimize
        if ns.gui.event_mouse_down(window.minmax_text) == ng.MC_LEFT then
            window.minimized = not window.minimized
        end
        ns.gui.set_visible(window.body, not window.minimized)
        ns.gui_text.set_str(window.minmax_text, window.minimized and '+' or '-')
    end

    -- move dragged window
    if drag_window then
        ns.transform.translate(drag_window, mouse_curr - mouse_prev)
    end

    mouse_prev = mouse_curr
end


--- textbox --------------------------------------------------------------------

ns.gui_textbox = { auto_saveload = true }

ns.gui_textbox.tbl = ng.entity_table()

function ns.gui_textbox.add(ent)
    if ns.gui_textbox.tbl[ent] then return end
    ns.gui_textbox.tbl[ent] = {}
    local gui_textbox = ns.gui_textbox.tbl[ent]

    -- add ent to gui_rect as container
    ng.add {
        ent = ent,
        gui_rect = {},
    }

    -- add text child
    gui_textbox.text = ng.add {
        transform = { parent = ent },
        gui = {
            color = ng.color_white,
            valign = ng.GA_MAX,
            halign = ng.GA_MIN
        },
        gui_text = {},
    }
end

function ns.gui_textbox.remove(ent)
    local textbox = ns.gui_textbox.tbl[ent]
    if textbox then ns.transform.destroy_rec(ent) end
    ns.gui_textbox.tbl[ent] = nil
end

function ns.gui_textbox.has(ent)
    return ns.gui_textbox.tbl[ent] ~= nil
end

ng.simple_props(ns.gui_textbox, {
    click_focus = false
})

function ns.gui_textbox.get_text(ent)
    local gui_textbox = ns.gui_textbox.tbl[ent]
    if gui_textbox then return gui_textbox.text end
end

function ns.gui_textbox.update_all()
    for ent in pairs(ns.gui_textbox.tbl) do
        if ns.entity.destroyed(ent) then ns.gui_textbox.remove(ent) end
    end

    for ent, textbox in pairs(ns.gui_textbox.tbl) do
        if textbox.click_focus
        and ns.gui.event_mouse_up(ent) == ng.MC_LEFT then
            ns.gui.set_focus(textbox.text, true)
        end
    end
end


--- gui_checkbox ---------------------------------------------------------------

ns.gui_checkbox = { auto_saveload = true }

ns.gui_checkbox.tbl = ng.entity_table()

function ns.gui_checkbox.add(ent)
    if ns.gui_checkbox.tbl[ent] then return end

    local checkbox = {}
    ns.gui_checkbox.tbl[ent] = checkbox

    checkbox.checked = false

    ns.gui_textbox.add(ent)
    checkbox.ent = ent
    checkbox.text = ns.gui_textbox.get_text(ent)
end

function ns.gui_checkbox.remove(ent)
    ns.gui_textbox.remove(ent)
    ns.gui_checkbox.tbl[ent] = nil
end

function ns.gui_checkbox.has(ent)
    return ns.gui_checkbox.tbl[ent] ~= nil
end

local function checkbox_toggle(checkbox)
    checkbox.checked = not checkbox.checked
    ns.gui.fire_event_changed(checkbox.ent)
end
function ns.gui_checkbox.toggle(ent)
    local checkbox = ns.gui_checkbox.tbl[ent]
    if checkbox then checkbox_toggle(checkbox) end
end
function ns.gui_checkbox.set_checked(ent, checked)
    local checkbox = ns.gui_checkbox.tbl[ent]
    -- do it this way to fire 'changed' event correctly
    if checkbox.checked ~= checked then checkbox_toggle(checkbox) end
end
function ns.gui_checkbox.get_checked(ent, checked)
    local checkbox = ns.gui_checkbox.tbl[ent]
    if checkbox then return checkbox.checked end
end

function ns.gui_checkbox.update_all(ent)
    for ent in pairs(ns.gui_checkbox.tbl) do
        if ns.entity.destroyed(ent) then ns.gui_checkbox.remove(ent) end
    end

    for ent, checkbox in pairs(ns.gui_checkbox.tbl) do
        if ns.gui.event_mouse_down(ent) == ng.MC_LEFT then
            checkbox_toggle(checkbox)
        end

        if checkbox.checked then
            ns.gui_text.set_str(checkbox.text, 'yes')
        else
            ns.gui_text.set_str(checkbox.text, 'no')
        end
    end
end
