ns.gui_test = {
    ent = nil
}

-- mu = neko.ui

local function make_color(tab)
    return {
        r = mu.ref(tab[1]),
        g = mu.ref(tab[2]),
        b = mu.ref(tab[3]),
        a = mu.ref(tab[4])
    }
end

local function uint8_slider(label, palette, key)
    mu.push_id(label .. key)
    mu.slider(palette[key], 0, 255, 0, "%.0f")
    mu.pop_id()
end

function write_log(str)
    local log = logbuf:get()
    if #log ~= 0 then
        log = log .. "\n"
    end
    log = log .. str
    logbuf:set(log)
    logbuf_updated = true
end

function test_lui()
    test_window()
    -- log_window()
    style_window()
end

function test_window()
    -- do window
    if mu.begin_window("Demo Window", mu.rect(40, 40, 300, 450)) then
        local win = mu.get_current_container()
        local rect = win:rect()
        win:set_rect{
            x = rect.x,
            y = rect.y,
            w = math.max(rect.w, 240),
            h = math.max(rect.h, 300)
        }

        -- window info
        if mu.header "Window Info" then
            local win = mu.get_current_container()
            local rect = win:rect()
            mu.layout_row({54, -1}, 0)
            mu.label "Position:"
            mu.label(("%d, %d"):format(rect.x, rect.y))
            mu.label "Size:"
            mu.label(("%d, %d"):format(rect.w, rect.h))
        end

        -- labels + buttons
        if mu.header("Test Buttons", mu.OPT_EXPANDED) then
            mu.layout_row({86, -110, -1}, 0)
            mu.label "Test buttons 1:"
            if mu.button "Button 1" then
                write_log "Pressed button 1"
            end
            if mu.button "Button 2" then
                write_log "Pressed button 2"
            end
            mu.label "Test buttons 2:"
            if mu.button "Button 3" then
                write_log "Pressed button 3"
            end
            if mu.button "Popup" then
                mu.open_popup "Test Popup"
            end
            if mu.begin_popup "Test Popup" then
                mu.button "Hello"
                mu.button "World"
                mu.end_popup()
            end
        end

        -- tree
        if mu.header("Tree and Text", mu.OPT_EXPANDED) then
            mu.layout_row({140, -1}, 0)
            mu.layout_begin_column()
            if mu.begin_treenode "Test 1" then
                if mu.begin_treenode "Test 1a" then
                    mu.label "Hello"
                    mu.label "World"
                    mu.end_treenode()
                end
                if mu.begin_treenode "Test 1b" then
                    if mu.button "Button 1" then
                        write_log "Pressed button 1"
                    end
                    if mu.button "Button 2" then
                        write_log "Pressed button 2"
                    end
                    mu.end_treenode()
                end
                mu.end_treenode()
            end
            if mu.begin_treenode "Test 2" then
                mu.layout_row({54, 54}, 0)
                if mu.button "Button 3" then
                    write_log "Pressed button 3"
                end
                if mu.button "Button 4" then
                    write_log "Pressed button 4"
                end
                if mu.button "Button 5" then
                    write_log "Pressed button 5"
                end
                if mu.button "Button 6" then
                    write_log "Pressed button 6"
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

            mu.layout_begin_column()
            mu.layout_row({-1}, 0)
            mu.text(
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus ipsum, eu varius magna felis a nulla.")
            mu.layout_end_column()
        end

        -- background color sliders
        if mu.header("Background Color", mu.OPT_EXPANDED) then
            mu.layout_row({-78, -1}, 74)
            mu.layout_begin_column()
            mu.layout_row({46, -1}, 0)
            mu.label "Red:";
            mu.slider(bg.r, 0, 255)
            mu.label "Green:";
            mu.slider(bg.g, 0, 255)
            mu.label "Blue:";
            mu.slider(bg.b, 0, 255)
            mu.layout_end_column()

            local r = mu.layout_next()
            local col = {
                r = math.floor(bg.r:get()),
                g = math.floor(bg.g:get()),
                b = math.floor(bg.b:get()),
                a = 255
            }
            -- neko.clear_color(col.r, col.g, col.b, col.a)
            mu.draw_rect(r, col)
            local str = ("#%02X%02X%02X"):format(col.r, col.g, col.b)
            mu.draw_control_text(str, r, mu.COLOR_TEXT, mu.OPT_ALIGNCENTER)
        end

        mu.end_window()
    end
end

function log_window()
    if mu.begin_window("Log Window", mu.rect(350, 40, 300, 200)) then
        -- output text panel
        mu.layout_row({-1}, -25)
        mu.begin_panel "Log Output"
        local panel = mu.get_current_container()
        mu.layout_row({-1}, -1)
        mu.text(logbuf:get())
        mu.end_panel()
        if logbuf_updated then
            local sx, sy = panel:scroll()
            local x, y = panel:content_size()
            panel:set_scroll(sx, y)
            logbuf_updated = false
        end

        -- input textbox + buttons
        local submitted = false
        mu.layout_row({-120, -60, -1}, 0)
        if (mu.textbox(log_input) and mu.RES_SUBMIT) ~= 0 then
            mu.set_focus(mu.get_last_id())
            submitted = true
        end
        if mu.button "Submit" then
            submitted = true
        end
        if mu.button "Clear" then
            logbuf:set ""
        end
        if submitted then
            write_log(log_input:get())
            log_input:set ""
        end

        mu.end_window()
    end
end

function style_window()
    if mu.begin_window("Style Editor", mu.rect(350, 250, 300, 240)) then
        local style = mu.get_style()

        local sw = mu.get_current_container():body().w * 0.14
        mu.layout_row({80, sw, sw, sw, sw, -1}, 0)
        for _, color in ipairs(ns.gui_test.colors) do
            local label, idx, palette = color[1], color[2], color[3]
            mu.label(label)
            uint8_slider(label, palette, "r")
            uint8_slider(label, palette, "g")
            uint8_slider(label, palette, "b")
            uint8_slider(label, palette, "a")
            local c = {
                r = palette.r:get(),
                g = palette.g:get(),
                b = palette.b:get(),
                a = palette.a:get()
            }
            mu.draw_rect(mu.layout_next(), c)
            style:set_color(idx, c)
        end
        mu.end_window()
    end
end

function ns.gui_test.update_all()
end

function ns.gui_test.add(ent)
    print("ns.gui_test.add", ent)
    ns.gui_test.colors = {{"text:", mu.COLOR_TEXT, make_color {230, 230, 230, 255}},
                          {"border:", mu.COLOR_BORDER, make_color {25, 25, 25, 255}},
                          {"windowbg:", mu.COLOR_WINDOWBG, make_color {50, 50, 50, 255}},
                          {"titlebg:", mu.COLOR_TITLEBG, make_color {25, 25, 25, 255}},
                          {"titletext:", mu.COLOR_TITLETEXT, make_color {240, 240, 240, 255}},
                          {"panelbg:", mu.COLOR_PANELBG, make_color {0, 0, 0, 0}},
                          {"button:", mu.COLOR_BUTTON, make_color {75, 75, 75, 255}},
                          {"buttonhover:", mu.COLOR_BUTTONHOVER, make_color {95, 95, 95, 255}},
                          {"buttonfocus:", mu.COLOR_BUTTONFOCUS, make_color {115, 115, 115, 255}},
                          {"base:", mu.COLOR_BASE, make_color {30, 30, 30, 255}},
                          {"basehover:", mu.COLOR_BASEHOVER, make_color {35, 35, 35, 255}},
                          {"basefocus:", mu.COLOR_BASEFOCUS, make_color {40, 40, 40, 255}},
                          {"scrollbase:", mu.COLOR_SCROLLBASE, make_color {43, 43, 43, 255}},
                          {"scrollthumb:", mu.COLOR_SCROLLTHUMB, make_color {30, 30, 30, 255}}}

    log_input = mu.ref ""

    logbuf = mu.ref ""
    logbuf_updated = false

    bg = {
        r = mu.ref(90),
        g = mu.ref(95),
        b = mu.ref(100)
    }
    checks = map({true, false, true}, mu.ref)

    ns.gui_test.ent = ent
end

function ns.gui_test.has(ent)
    print("ns.gui_test.has", ent)
    return ns.gui_test.ent ~= nil
end

function ns.gui_test.draw_ui()
    if ns.gui_test.ent ~= nil then
        test_lui()
    end

    draw_imgui()
end

draw_imgui = function(dt)
    local ImGui = imgui
    ImGui.Begin("Demo")

    local ImGuiEnd<close> = setmetatable({}, {
        __close = function()
            ImGui.End()
        end
    })

    ImGui.Text("TestDemo")

    if ImGui.Button("test_http") then
        local http = require 'http'

        local status, data = http.request('https://www.7ed.net/ruozi/api')

        print('welcome')
        print(status)
        print(data)

        local json_data = neko.json_read(common.decode_unicode_escape(data))

        print(table.show(json_data))

        print(neko.json_write(json_data))
    end

    if ImGui.Button("build_pack") then
        neko.bindata_build(("fgd.pack"), {"assets/fmod/Build/Desktop/Master.bank", "neko_base.fgd"})
    end

    if ImGui.Button("read_pack") then
        local test_pack, test_handle, test_items

        local test_pack_buildnum, test_pack_item_count = neko.bindata_info("fgd.pack")
        print("pack_info", test_pack_buildnum, test_pack_item_count)
        test_pack = neko.bindata_load("test_pack_handle", "fgd.pack")
        test_handle = test_pack:assets_load("neko_base.fgd")
        test_items = test_pack:items()
        print(type(test_handle))
        test_pack:assets_unload(test_handle)
        print(table.show(test_items))
    end

    if ImGui.Button("test_reg") then
        local reg = neko.from_registry("_PRELOAD")
        dump_func(reg)

        reg = neko.from_registry(LUA_RIDX_GLOBALS, "sandbox")
        dump_func(reg)
    end

    if ImGui.Button("test_traceback") then

        local function bar(f1, f2)
            print(neko.traceback(10, 10, true, true))
        end
        local function foo(a, b, c)
            bar(print, foo)
        end
        foo("s", {}, 234)
    end

    if ImGui.Button("test_ecs") then

        print(table.show({gw:detail()}))
    end

    if ImGui.Button("test_callback") then

        local f = function(f1, f2)
            print(neko.traceback(10, 10, true, true))
        end

        neko.callback_save("test1", f)

        neko.callback_call("test1", "haha", 114514)

    end

    -- ImGui.Checkbox(neko.conf.cvar.shader_inspect)

    ImGui.Separator()

    if ImGui.Button("thread") then
        local c1 = neko.make_channel 'c1'
        local c2 = neko.make_channel 'c2'

        local t1 = neko.make_thread [[
          local c1 = neko.get_channel 'c1'
          neko.thread_sleep(1)
          c1:send 'one'
        ]]

        local t2 = neko.make_thread [[
          local c2 = neko.get_channel 'c2'
          neko.thread_sleep(2)
          c2:send 'two'
        ]]

        for i = 1, 2 do
            local msg, ch = neko.select(c1, c2)
            print(msg, ch)
        end
    end

    -- if ImGui.InputText("TEST", text) then
    --     print(tostring(text))
    -- end

    -- ImGui.Image(test_tex, 100.0, 100.0)
    -- ImGui.Image(test_custom_sprite.tex, 100.0, 100.0)

end
