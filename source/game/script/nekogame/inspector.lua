ns.inspector = {}

-- display gui entity
local display = ng.add {
    group = {
        groups = 'builtin'
    }
}

function ns.inspector.draw_all()
    Inspector.inspector_draw(Inspector.inspector_get())
    -- lovebird.update()
    -- game_imgui.ShowDemoWindow()

    if game_imgui.Begin("Test") then
        game_imgui.Text("hello你好世界")

        if game_imgui.Button("test") then
            local mmm = hot_require("libs/mm")
            mmm(package.preload)

            print("haha")
        end

        for name, _ in pairs(package.loaded) do
            game_imgui.Text(name)
        end

        game_imgui.End()
    end
end
