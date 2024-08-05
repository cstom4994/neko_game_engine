--- layout ---------------------------------------------------------------------
ns.edit.bottom_rect = ng.add {
    transform = {
        parent = ns.edit.gui_root
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true
    },
    gui = {
        color = ng.color(0, 0, 0, 0), -- invisible
        captures_events = false,
        halign = ng.GA_MIN,
        valign = ng.GA_MIN,
        padding = ng.vec2_zero
    }
}

ns.edit.status_bar = ng.add {
    transform = {
        parent = ns.edit.bottom_rect
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true
    },
    gui = {
        color = ng.color(0.8, 0.3, 0.3, 1.0),
        halign = ng.GA_MIN,
        valign = ng.GA_TABLE,
        padding = ng.vec2_zero
    }
}
ng.add {
    transform = {
        parent = ns.edit.status_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_text = {
        str = ''
    },
    gui = {
        color = ng.color_black,
        halign = ng.GA_MIN,
        valign = ng.GA_MAX,
        padding = ng.vec2(2, 2)
    }
}

ns.edit.bottom_bar = ng.add {
    transform = {
        parent = ns.edit.bottom_rect
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true
    },
    gui = {
        color = ng.color(0.9, 0.9, 0.9, 1.0),
        halign = ng.GA_MIN,
        valign = ng.GA_TABLE,
        padding = ng.vec2_zero
    }
}
ng.add {
    transform = {
        parent = ns.edit.bottom_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_text = {
        str = ''
    },
    gui = {
        color = ng.color_black,
        halign = ng.GA_MIN,
        valign = ng.GA_MAX,
        padding = ng.vec2(2, 2)
    }
}

--- text -----------------------------------------------------------------------

local function create_status_textbox(gap, label)
    local textbox = ng.add {
        transform = {
            parent = ns.edit.status_bar
        },
        group = {
            groups = 'builtin'
        },
        edit = {
            editable = false
        },
        gui = {
            color = ng.color(0.6, 0.1, 0.1, 1),
            halign = ng.GA_TABLE,
            valign = ng.GA_MAX,
            padding = ng.vec2(gap, 0)
        },
        gui_rect = {}
    }
    local text = ng.add {
        transform = {
            parent = textbox
        },
        group = {
            groups = 'builtin'
        },
        edit = {
            editable = false
        },
        gui_text = {
            str = label or ''
        },
        gui = {
            color = ng.color_white,
            halign = ng.GA_MIN,
            valign = ng.GA_MAX,
            padding = ng.vec2(4, 2)
        }
    }
    return textbox, text
end

ns.edit.edit_textbox, ns.edit.edit_text = create_status_textbox(0, 'edit')
ns.edit.grid_textbox, ns.edit.grid_text = create_status_textbox(7, '')
ns.edit.select_textbox, ns.edit.select_text = create_status_textbox(7, '')
ns.edit.mode_textbox, ns.edit.mode_text = create_status_textbox(7, '')

ns.edit.play_textbox, ns.edit.play_text = create_status_textbox(0, '\xcb')
ns.gui.set_halign(ns.edit.play_textbox, ng.GA_MAX)

function ns.edit.set_mode_text(s)
    ns.gui.set_visible(ns.edit.mode_textbox, true)
    ns.gui_text.set_str(ns.edit.mode_text, s)
end
function ns.edit.hide_mode_text()
    ns.gui.set_visible(ns.edit.mode_textbox, false)
end
ns.edit.hide_mode_text()

--- command text ---------------------------------------------------------------

ns.edit.command_bar = ng.add {
    transform = {
        parent = ns.edit.bottom_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_rect = {
        hfill = true
    },
    gui = {
        visible = false,
        color = ng.color_clear,
        halign = ng.GA_MIN,
        valign = ng.GA_TABLE,
        padding = ng.vec2_zero
    }
}
ns.edit.command_text_colon = ng.add {
    transform = {
        parent = ns.edit.command_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui = {
        color = ng.color_black,
        halign = ng.GA_TABLE,
        valign = ng.GA_MAX,
        padding = ng.vec2(2, 2)
    },
    gui_text = {
        str = ':'
    }
}
ns.edit.command_text = ng.add {
    transform = {
        parent = ns.edit.command_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui = {
        color = ng.color_black,
        halign = ng.GA_TABLE,
        valign = ng.GA_MAX,
        padding = ng.vec2(0, 2)
    },
    gui_text = {
        str = ''
    },
    gui_textedit = {}
}
ns.edit.command_completions_text = ng.add {
    transform = {
        parent = ns.edit.command_bar
    },
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui = {
        color = ng.color(0.5, 0.30, 0.05, 1.0),
        halign = ng.GA_TABLE,
        valign = ng.GA_MAX,
        padding = ng.vec2(0, 2)
    },
    gui_text = {
        str = 'completion1 | completion2'
    },
    gui_textedit = {}
}
