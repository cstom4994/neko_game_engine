ns.inspector = {}

-- display gui entity
local display = ng.add {
    group = {
        groups = 'builtin'
    }
}

function ns.inspector.draw_all()
    -- Inspector.inspector_draw(Inspector.inspector_get())
    -- lovebird.update()
    -- game_imgui.ShowDemoWindow()

    if game_imgui.Begin("Test") then
        game_imgui.Text("hgaha")
        game_imgui.End()
    end
end
