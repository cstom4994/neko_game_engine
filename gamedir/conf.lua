function __neko_conf(t)
    t.app = {
        title = "ahaha",
        width = 1280.0,
        height = 720.0,
        game_proxy = "default",
        default_font = "@gamedata/assets/fonts/VonwaonBitmap-16px.ttf",
        dump_allocs_detailed = true,
        swap_interval = 1,
        target_fps = 120,
        reload_interval = 1,
        debug_on = false,
        batch_vertex_capacity = 2048
    }
    t.vfs = {
        luacode = {
            path = "../source/game",
            redirect = ""
        },
        gamedata = {
            path = "../gamedir",
            redirect = ""
        },
        -- demo_data = {
        --     path = "../demo_data.lua",
        --     redirect = "luabp/source/game"
        -- }
        -- luacode = {
        --     path = "gamedir.lua",
        --     redirect = "luabp/source/game"
        -- },
        -- gamedata = {
        --     path = "gamedir.lua",
        --     redirect = "luabp/gamedir"
        -- },
    }
end
