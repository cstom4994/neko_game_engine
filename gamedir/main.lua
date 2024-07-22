function neko.conf(t)
    t.swap_interval = 1
    t.target_fps = 120
    t.reload_interval = 0.001
    t.window_width = 1280
    t.window_height = 720
    -- t.imgui_font = "assets/fonts/fusion-pixel-12px-monospaced-zh_hans.ttf"
end

function neko.before_quit()
    neko_proxy_before_quit()
end

function neko.start(arg)
    neko_proxy_start(arg)
end

function neko.frame(dt)
    neko_proxy_frame(dt)
end
