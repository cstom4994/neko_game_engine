ns.inspector = {}

-- display gui entity
local display = ng.add {
    group = {
        groups = 'builtin'
    }
}

local test_ase = neko.sprite_load("assets/B_witch.ase")

local bg = {
    r = neko.ui.ref(90),
    g = neko.ui.ref(95),
    b = neko.ui.ref(100)
}

local function map(arr, fn)
    local t = {}
    for k, v in pairs(arr) do
        t[k] = fn(v)
    end
    return t
end

local checks = map({true, false, true}, neko.ui.ref)

function ns.inspector.draw_all()

    local dt = ns.timing.instance.dt

    test_ase:update(dt)
    local ox = test_ase:width() / 2
    local oy = test_ase:height()
    test_ase:draw(200, 200, 0, 1 * 3.9, 1 * 3.9, ox, oy)
end

function ns.inspector.draw_ui()

    Inspector.inspector_draw(Inspector.inspector_get())

    if mu.begin_window("UI Test Window", mu.rect(40, 40, 300, 450)) then

        mu.label "Test:"

        -- local win = mu.get_current_container()
        -- local rect = win:rect()
        -- win:set_rect{
        --     x = rect.x,
        --     y = rect.y,
        --     w = math.max(rect.w, 240),
        --     h = math.max(rect.h, 300)
        -- }

        -- -- window info
        if mu.header "Window Info" then
            local win = mu.get_current_container()
            local rect = win:rect()
            -- mu.layout_row({54, -1}, 0)
            mu.label "Position:"
            mu.label(("%d, %d"):format(rect.x, rect.y))
            mu.label "Size:"
            mu.label(("%d, %d"):format(rect.w, rect.h))
        end

        if mu.button("Test") then
            mm(ng)
            print(Core.ltype(ng))
        end

        if mu.button("test_http") then
            local http = require 'http'

            local status, data = http.request('https://www.7ed.net/ruozi/api')

            print('welcome')
            print(status)
            print(data)

            local json_data = ng.api.json_read(common.decode_unicode_escape(data))

            dump_func(json_data)

            print(ng.api.json_write(json_data))
        end

        -- mu.layout_row({140, -1}, 0)
        mu.layout_begin_column()
        if mu.begin_treenode "Test 1" then
            if mu.begin_treenode "Test 1a" then
                mu.label "Hello"
                mu.label "World"
                mu.end_treenode()
            end
            if mu.begin_treenode "Test 1b" then
                if mu.button "Button 1" then
                end
                mu.end_treenode()
            end
            mu.end_treenode()
        end
        if mu.begin_treenode "Test 3" then
            mu.checkbox("Checkbox 1", checks[1])
            mu.checkbox("Checkbox 2", checks[2])
            mu.checkbox("Checkbox 3", checks[3])
            mu.end_treenode()
        end
        mu.layout_end_column()

        for name, _ in pairs(package.loaded) do
            mu.label(name)
        end

        mu.end_window()
    end

end
