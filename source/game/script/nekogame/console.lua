-- create console gui in top-left corner
local console_background = ng.add {
    group = { groups = 'builtin' },
    edit = { editable = false },
    gui_rect = { hfill = true },
    gui = {
        color = ng.color(1, 1, 1, 0.5),
        padding = ng.vec2_zero,
        visible = false,
    },
}
local console_text = ng.add {
    transform = { parent = console_background },
    group = { groups = 'builtin' },
    edit = { editable = false },
    gui = {
        color = ng.color_black,
        halign = ng.GA_MIN, valign = ng.GA_MAX,
        padding = ng.vec2(1, 1),
    },
}
ns.console.set_entity(console_text)

-- rebind Lua 'print' to print to console
function print(...)
    local args = table.pack(...)
    local sargs = {}
    for i = 1, args.n do
        sargs[i] = tostring(args[i])
    end
    ns.console.puts(table.concat(sargs, '    '))
end
