function neko.conf(t)

    t.app = {
        title = "ahaha",
        width = 1280.0,
        height = 720.0,
        game_proxy = "default",
        default_font = "assets/fonts/VonwaonBitmap-16px.ttf",
        dump_allocs_detailed = false,
        swap_interval = 1,
        target_fps = 120,
        reload_interval = 0.001,
        debug_on = false,
        batch_vertex_capacity = 2048
    }
end
