function haha()

    LocalGame = {}

    hot_require 'oscillator'
    hot_require 'rotator'

    ns.sprite.set_atlas('assets/atlas.ase')

    -- add camera: 32 pixels is one unit

    LocalGame.camera = ns.entity.create()

    ns.transform.add(LocalGame.camera)
    ns.camera.add(LocalGame.camera)
    ns.camera.set_viewport_height(LocalGame.camera, 120)

    -- 添加一些block

    local block = ng.add {
        transform = {
            position = ng.vec2(0, 0),
            scale = ng.vec2(20, 20)
        },
        sprite = {
            texcell = ng.vec2(32, 32),
            texsize = ng.vec2(32, 32)
        }
    }

    -- ns.oscillator.add(block, {
    --     amp = 1,
    --     freq = 1,
    --     phase = 5
    -- })
    ns.rotator.add(block, 1 * math.pi)

    local level_1 = ng.add {
        transform = {
            position = ng.vec2(0, 0),
            scale = ng.vec2(2, 2)
        },
        tiled = {
            map = "assets/maps/map.tmx"
        }
    }

    -- local player_ent = ng.add {
    --     transform = {
    --         position = ng.vec2(0, 0),
    --         scale = ng.vec2(0, 0)
    --     },
    --     player = {}
    --     -- aseprite = {
    --     --     ase = "assets/B_witch.ase"
    --     -- }
    -- }

    -- keyboard_controlled_add(ng.cent(player_ent))
    -- keyboard_controlled_set_v(ng.cent(player_ent), 100.0)

    local ffi = FFI

    ffi.cdef [[
    typedef struct {
        float x;
        float y;
        float z;
        float w;
    } neko_vec4_t;
    typedef struct {
        uint32_t id;
    } __lua_tex_t;
    typedef struct {
        float v[16];
    } __lua_quad_vdata_t;
    typedef struct {
        uint32_t v[6];
    } __lua_quad_idata_t;
    ]]

    local function Npc_load_prefab(self)
        local prefab_tb
        -- local file_content = read_file("data/" .. self.name .. ".neko")

        local game_db = db.open("test/data.luadb")

        local file_content = game_db.all[self.name]
        if file_content then
            prefab_tb = common.prefabs.load(file_content)
            assert(common.prefabs.check(prefab_tb))
            assert(common.prefabs.node_type(prefab_tb) == "npc")
        else
            print("无法打开文件或文件不存在", self.name)
        end

        local sprite_res = prefab_tb[2]["sprite"]

        -- self.sprite = neko.sprite_load(sprite_res)

        self.facing_left = prefab_tb[2]["facing_left"]
        self.was_hit = prefab_tb[2]["was_hit"]
        self.hp = prefab_tb[2]["hp"]
        self.hp_max = prefab_tb[2]["hp_max"]

        -- if prefab_tb[2]["spring"] then
        --     self.spring = Spring()
        -- end
        -- self.sprite:play "skel_run"
        -- self.hpbar = Hpbar(self)
        -- self.type = "enemy"
    end

    function neko.__define_default_callbacks()

        print("haha __define_default_callbacks")

        luainspector = neko.core.inspector_init()

        local test_ase

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

        neko.args = function(args)
        end

        neko.game_init_thread = function()
        end
        neko.game_init = function()
            test_ase = neko.sprite_load("assets/B_witch.ase")

            LocalGame.b2 = neko.b2_world {
                gx = 0,
                gy = 0,
                meter = 16
            }

            LocalGame.world = World()

            LocalGame.world:add(CPlayer(0, 0))

        end
        neko.game_fini = function()
        end
        neko.game_pre_update = function()
        end
        neko.game_loop = function(dt)

            LocalGame.b2:step(dt)
            LocalGame.world:update(dt)

        end

        neko.game_render = function()

            LocalGame.world:draw()

            local dt = ng.get_timing_instance().dt

            test_ase:update(dt)
            local ox = test_ase:width() / 2
            local oy = -test_ase:height()
            test_ase:draw(0, 0, 0, 1, -1, ox, oy)
        end

        neko.game_ui = function()

            -- print("haha")

            neko.core.inspector_draw(neko.core.inspector_get())

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
                    -- print(ng)
                    print(ng.api.core.ltype(player_ent))
                end

                if mu.button("Test event") then
                    print(event)
                end

                if mu.button("Test luadb") then
                    local mydb = {
                        name = "Skeleton"
                    }
                    Npc_load_prefab(mydb)
                    print(mydb)
                end

                if mu.button("test_http") then
                    local http = require 'http'

                    local status, data = http.request('https://www.7ed.net/ruozi/api')

                    print('welcome')
                    print(status)
                    print(data)

                    local json_data = ng.api.json_read(common.decode_unicode_escape(data))

                    print(table.show(json_data))

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

        neko.before_quit = function()
            print("before_quit")
        end

    end

    --[[
    function neko.__define_default_callbacks()
    
    
        local TEX_WIDTH = 128
        local TEX_HEIGHT = 128
    
        local gd = {}
    
    
    
        neko.game_init = function()
            print("neko.game_init")
    
    
            local win = ng.game_get_window_size()
            local win_w, win_h = win.x, win.y
    
            gd.main_fbo = ng.api.core.render_framebuffer_create()
            gd.main_rt = ng.api.core.render_texture_create(win_w, win_h, {
                type = "R_TEXTURE_2D",
                format = "R_TEXTURE_FORMAT_RGBA32F",
                wrap_s = "R_TEXTURE_WRAP_REPEAT",
                wrap_t = "R_TEXTURE_WRAP_REPEAT",
                min_filter = "R_TEXTURE_FILTER_NEAREST",
                mag_filter = "R_TEXTURE_FILTER_NEAREST"
            })
            gd.main_rp = ng.api.core.render_renderpass_create(gd.main_fbo, gd.main_rt)
    
            gd.fbo = ng.api.core.render_framebuffer_create()
            gd.rt = ng.api.core.render_texture_create(win_w, win_h)
            gd.rp = ng.api.core.render_renderpass_create(gd.fbo, gd.rt)
            
            gd.rp_default = ng.api.core.render_renderpass_default()
    
            -- gd.fontbatch = ng.api.core.fontbatch_create(font_vs, font_ps)
    
            -- test_shader = ng.api.core.render_shader_create("compute", {
            --     -- VERTEX = sprite_vs,
            --     -- FRAGMENT = sprite_fs
            --     COMPUTE = comp_src
            -- })
    
            test_uniform = ng.api.core.render_uniform_create("u_roll", {{
                type = "R_UNIFORM_FLOAT"
            }})
    
            test_tex = ng.api.core.render_texture_create(TEX_WIDTH, TEX_HEIGHT, {
                type = "R_TEXTURE_2D",
                format = "R_TEXTURE_FORMAT_RGBA32F",
                wrap_s = "R_TEXTURE_WRAP_REPEAT",
                wrap_t = "R_TEXTURE_WRAP_REPEAT",
                min_filter = "R_TEXTURE_FILTER_NEAREST",
                mag_filter = "R_TEXTURE_FILTER_NEAREST"
            })
    
            -- test_pipeline = ng.api.core.render_pipeline_create("test_pipeline", {
            --     compute = {
            --         shader = test_shader
            --     }
            -- })
    
            local test_vec4_data = ffi.new("neko_vec4_t")
    
            test_vec4_data.x = 0.0
            test_vec4_data.y = 1.0
            test_vec4_data.z = 1.0
            test_vec4_data.w = 1.0
    
            -- test_storage_buffer = ng.api.core.render_storage_buffer_create("u_voxels", test_vec4_data,
            --     ffi.sizeof("neko_vec4_t"))
    
            -- test_custom_sprite = {}
            -- test_custom_sprite.tex = ng.api.core.render_texture_create(10, 10, {
            --     type = "R_TEXTURE_2D",
            --     data = ng.api.core.gen_tex(),
            --     format = "R_TEXTURE_FORMAT_RGBA8",
            --     wrap_s = "R_TEXTURE_WRAP_REPEAT",
            --     wrap_t = "R_TEXTURE_WRAP_REPEAT",
            --     min_filter = "R_TEXTURE_FILTER_NEAREST",
            --     mag_filter = "R_TEXTURE_FILTER_NEAREST"
            -- })
    
            -- test_custom_sprite.tex_userdata = ffi.new("__lua_tex_t")
            -- test_custom_sprite.tex_userdata.id = test_custom_sprite.tex.id
    
            -- local v = {-0.5, -0.5, 0.0, 0.0, 0.5, -0.5, 1.0, 0.0, -0.5, 0.5, 0.0, 1.0, 0.5, 0.5, 1.0, 1.0}
            -- test_custom_sprite.vdata = ffi.new("__lua_quad_vdata_t")
    
            -- for i, v in ipairs(v) do
            --     test_custom_sprite.vdata.v[i] = v
            -- end
    
            -- v = {0, 3, 2, 0, 1, 3}
            -- test_custom_sprite.idata = ffi.new("__lua_quad_idata_t")
            -- for i, v in ipairs(v) do
            --     test_custom_sprite.idata.v[i] = v
            -- end
    
            -- test_custom_sprite.vbo = ng.api.core.render_vertex_buffer_create("vbo", test_custom_sprite.vdata,
            --     ffi.sizeof("__lua_quad_vdata_t"))
            -- test_custom_sprite.ibo = ng.api.core.render_index_buffer_create("ibo", test_custom_sprite.idata,
            --     ffi.sizeof("__lua_quad_idata_t"))
    
            -- test_custom_sprite.uniform = ng.api.core.render_uniform_create("u_tex", {{
            --     type = "R_UNIFORM_SAMPLER2D"
            -- }}, "R_SHADER_STAGE_FRAGMENT")
    
            -- test_custom_sprite.shader = ng.api.core.render_shader_create("quad", {
            --     VERTEX = custom_sprite_vs,
            --     FRAGMENT = custom_sprite_fs
            -- })
    
            -- test_custom_sprite.v_attr, test_custom_sprite.v_attr_size =
            --     ng.api.core.render_vertex_attribute_create("vertex_attribute_name", {{
            --         name = "a_pos",
            --         format = "R_VERTEX_ATTRIBUTE_FLOAT2"
            --     }, {
            --         name = "a_uv",
            --         format = "R_VERTEX_ATTRIBUTE_FLOAT2"
            --     }})
    
            -- test_custom_sprite.pipeline = ng.api.core.render_pipeline_create("test_pipeline", {
            --     raster = {
            --         shader = test_custom_sprite.shader,
            --         index_buffer_element_size = ffi.sizeof("uint32_t")
            --     },
            --     layout = {
            --         attrs = test_custom_sprite.v_attr,
            --         size = test_custom_sprite.v_attr_size
            --     }
            -- })
        end
    
        neko.game_fini = function()
            print("neko.game_fini")
    
            ng.api.core.render_renderpass_fini(gd.rp)
            ng.api.core.render_texture_fini(gd.rt)
            ng.api.core.render_framebuffer_fini(gd.fbo)
    
            ng.api.core.render_renderpass_fini(gd.main_rp)
            ng.api.core.render_texture_fini(gd.main_rt)
            ng.api.core.render_framebuffer_fini(gd.main_fbo)
        end
    
        neko.game_pre_update = function()
            -- print("neko.game_pre_update")
        end
    
        neko.game_loop = function(dt)
            -- print("neko.game_loop")
        end
    
        neko.game_render = function()
            -- print("neko.game_render")
    
    
    
            local t = ns.timing.get_elapsed()
    
            local fbs_x = 640
            local fbs_y = 360
    
            -- local win_w, win_h = neko_window_size(neko_main_window())
    
            local win_w, win_h = ng.api.core.render_display_size()
    
            -- mm({win_w, win_h})
    
            local roll = ns.timing.get_elapsed() * 0.001
    
            -- ng.api.core.render_pipeline_bind(test_pipeline)
            -- ng.api.core.render_apply_bindings({
            --     uniforms = {{
            --         uniform = test_uniform,
            --         data = roll
            --     }},
            --     image_buffers = {{
            --         tex = test_tex,
            --         binding = 0
            --     }},
            --     storage_buffers = {{
            --         buffer = test_storage_buffer,
            --         binding = 1
            --     }}
            -- })
            -- ng.api.core.render_dispatch_compute(TEX_WIDTH / 16, TEX_HEIGHT / 16, 1)
    
            -- local v1 = to_vec2(0.0, 0.0)
            -- local v2 = to_vec2(TEX_WIDTH, TEX_HEIGHT)
    
            -- ng.api.core.idraw_defaults()
            -- ng.api.core.idraw_camera2d(fbs_x, fbs_y)
    
            -- ng.api.core.idraw_texture(test_tex)
            -- ng.api.core.idraw_rectvd(v1, v2, to_vec2(0.0, 0.0), to_vec2(1.0, 1.0), "R_PRIMITIVE_TRIANGLES",
            --     to_color(255, 255, 255, 255))
    
            -- ng.api.core.render_renderpass_begin(gd.main_rp)
            -- ng.api.core.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
            -- ng.api.core.render_clear(0.0, 0.0, 0.0, 0.0)
            -- ng.api.core.idraw_draw()
            -- ng.api.core.render_renderpass_end()
    
            -- ng.api.core.render_renderpass_begin(gd.main_rp)
            -- ng.api.core.render_pipeline_bind(test_custom_sprite.pipeline)
            -- ng.api.core.render_apply_bindings({
            --     uniforms = {{
            --         uniform = test_custom_sprite.uniform,
            --         data = test_custom_sprite.tex_userdata,
            --         binding = 0
            --     }},
            --     vertex_buffers = {{
            --         buffer = test_custom_sprite.vbo
            --     }},
            --     index_buffers = {{
            --         buffer = test_custom_sprite.ibo
            --     }}
            -- })
            -- ng.api.core.render_draw({
            --     start = 0,
            --     count = 6
            -- })
            -- ng.api.core.render_renderpass_end()
    
            ng.api.core.idraw_defaults()
            ng.api.core.idraw_camera3d(fbs_x, fbs_y)
            ng.api.core.idraw_translatef(0.0, 0.0, -2.0)
            ng.api.core.idraw_rotatev(ns.timing.get_elapsed() * 0.0001, 1.0, 0.0, 0.0)
            ng.api.core.idraw_rotatev(ns.timing.get_elapsed() * 0.0001, 0.0, 1.0, 0.0)
            ng.api.core.idraw_rotatev(ns.timing.get_elapsed() * 0.0001, 0.0, 0.0, 1.0)
            ng.api.core.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 200, 100, 50, 255, "R_PRIMITIVE_LINES")
    
            ng.api.core.render_renderpass_begin(gd.rp)
            ng.api.core.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
            ng.api.core.render_clear(0.0, 0.0, 0.0, 1.0)
            ng.api.core.idraw_draw()
            ng.api.core.render_renderpass_end()
    
            ng.api.core.idraw_camera3d(fbs_x, fbs_y)
            ng.api.core.idraw_depth_enabled(true)
            ng.api.core.idraw_face_cull_enabled(true)
            ng.api.core.idraw_translatef(0.0, 0.0, -1.0)
            ng.api.core.idraw_texture(gd.rt)
            ng.api.core.idraw_rotatev(ns.timing.get_elapsed() * 0.0001, 1.0, 0.0, 0.0)
            ng.api.core.idraw_rotatev(ns.timing.get_elapsed() * 0.0002, 0.0, 1.0, 0.0)
            ng.api.core.idraw_rotatev(ns.timing.get_elapsed() * 0.0003, 0.0, 0.0, 1.0)
            ng.api.core.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 255, 255, 255, 255, "R_PRIMITIVE_TRIANGLES")
    
            ng.api.core.render_renderpass_begin(gd.rp_default)
            ng.api.core.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
            ng.api.core.idraw_draw()
            ng.api.core.render_renderpass_end()
    
            ng.api.core.idraw_defaults()
            ng.api.core.idraw_camera2d(fbs_x, fbs_y)
            ng.api.core.idraw_texture(gd.main_rt)
            ng.api.core.idraw_rectvd(to_vec2(0.0, 0.0), to_vec2(win_w, win_h), to_vec2(0.0, 0.0), to_vec2(1.0, 1.0),
                "R_PRIMITIVE_TRIANGLES", to_color(255, 255, 255, 255))
    
            ng.api.core.render_renderpass_begin(gd.rp_default)
            ng.api.core.render_set_viewport(0.0, 0.0, win_w, win_h)
            -- ng.api.core.idraw_draw()
            -- ng.api.core.render_clear(0.0, 0.0, 0.0, 1.0)
            -- ng.api.core.fontbatch_draw(gd.fontbatch)
            ng.api.core.render_renderpass_end()
    
        end
    
    end
    
    ]]

    print("haha.lua loaded")

end
