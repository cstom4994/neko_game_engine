ns.inspector = {}

-- display gui entity
local display = ng.add {
    group = {
        groups = 'builtin'
    }
}

function ns.inspector.draw_all()
    Inspector.inspector_draw(Inspector.inspector_get())
end

function ns.inspector.draw_ui()

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
            mu.layout_row({54, -1}, 0)
            mu.label "Position:"
            mu.label(("%d, %d"):format(rect.x, rect.y))
            mu.label "Size:"
            mu.label(("%d, %d"):format(rect.w, rect.h))
        end

        if mu.button("Test") then
            -- local mmm = hot_require("libs/mm")
            -- mmm(package.preload)
            mm(mu)
            print("haha")
        end

        for name, _ in pairs(package.loaded) do
            mu.label(name)
        end

        mu.end_window()
    end

end
