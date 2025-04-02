function ImGuiMouseHoldon()
    local ImGui = neko.imgui_obsolete
    if ImGui.IsItemHovered() then
        if ImGui.IsMouseDown(0) then
            return true
        end
    end
    return false
end

function haha()
    LocalGame = {}

    local default_font = neko.font_load("default")

    neko.set_master_volume(0.2)

    EcsWorld:register("test_component_1", {
        x = 0,
        y = 0
    })

    local test_component_1_ent1 = EcsWorld:new{
        test_component_1 = {
            x = 114514,
            y = 233
        }
    }

    ns.gamelogic = {
        pool = {}
    }

    local GameLogic = ns.gamelogic

    hot_require 'oscillator'
    hot_require 'rotator'

    ns.sprite.set_atlas('assets/atlas.ase')

    -- add camera: 32 pixels is one unit

    LocalGame.camera = ns.entity.create("Main Camera")

    ns.transform.add(LocalGame.camera)
    ns.camera.add(LocalGame.camera)
    ns.camera.set_viewport_height(LocalGame.camera, 120)

    ns.camera.set_current_camera(LocalGame.camera)

    -- 添加一些block

    local block = ng.add {
        transform = {
            position = ng.Vec2(0, 0),
            scale = ng.Vec2(20, 20)
        },
        sprite = {
            texcell = ng.Vec2(32, 32),
            texsize = ng.Vec2(32, 32)
        }
    }

    -- ns.oscillator.add(block, {
    --     amp = 1,
    --     freq = 1,
    --     phase = 5
    -- })
    ns.rotator.add(block, 1 * math.pi)

    ns.edit_inspector.add(block, "transform")
    ns.edit_inspector.add(block, "sprite")
    -- ns.edit_inspector.add(block, "tiled")

    print("blockblockblock", block.id)

    local game_tick = 0

    draw_fixtures = false

    player = {}

    local ttttttttt = ng.add {
        -- gui_test = {}
    }

    -- local player_ent = ng.add {
    --     transform = {
    --         position = ng.Vec2(0, 0),
    --         scale = ng.Vec2(0, 0)
    --     },
    --     player = {},
    --     aseprite = {
    --         ase = "assets/B_witch.ase"
    --     }
    -- }

    local function random_spawn_npc(n, f)
        n = n or 8
        f = f or function(v)
        end
        for i = 1, n do
            local v = {
                -- id = choose({"CEnemy"}),
                id = "CEnemy",
                x = random(0, 360),
                y = random(-360, 0)
            }

            local dist = distance(v.x, v.y, player.x, player.y)
            if dist < 100 then
                break
            end

            local mt = _G[v.id]
            if mt ~= nil then
                local obj = LocalGame.world:add(mt(v.x, v.y, choose({"chort", "skel", "zb"})))
                if v.id == "CPlayer" then
                    player = obj
                end
                -- print(v.id, v.x, v.y)
            else
                print("no " .. v.id .. " class exists")
            end

            f(v)
        end
    end

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

    print("haha __define_default_callbacks")

    local level_1 = ng.add {
        transform = {
            position = ng.Vec2(0, 0),
            scale = ng.Vec2(2, 2)
        },
        tiled = {
            map = "assets/maps/map.tmx"
        }
    }

    LocalGame.level_1 = level_1

    LocalGame.level_1_obj = ns["tiled"].get_obj(LocalGame.level_1)

    print(LocalGame.level_1_obj)

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

    function hit_player(demage)
        player.hp = player.hp - demage
    end

    ns.gamelogic.init = function()

        LocalGame.score = 0
        LocalGame.win_size = neko.window_size()

        LocalGame.b2 = neko.b2_world {
            gx = 0,
            gy = 0,
            meter = 16
        }

        LocalGame.world = World()

        player = CPlayer(0, 0)

        LocalGame.world:add(player)
        LocalGame.world:add(CEnemy(20, 20))

        cursor = Cursor(neko.sprite_load "assets/cursor.ase")

        bow_img = neko.sprite_load "assets/bow.ase"
        arrow_img = neko.sprite_load "assets/arrow.ase"

        local objs = LocalGame.level_1_obj
        for object_group_name, object_groups in pairs(objs) do
            for object_name, objects in pairs(object_groups) do
                local OBJ = objs[object_group_name][object_name]
                -- print(OBJ)
                if type(OBJ) == "table" then
                    local x = OBJ.x
                    local y = -OBJ.y
                    local class_name = OBJ.class_name
                    if class_name == "game_ent" then
                        local properties = OBJ.properties
                        if properties.ent_name ~= nil then
                            local mt = _G[properties.ent_name]
                            if mt ~= nil then
                                local new_obj = LocalGame.world:add(mt(x, y))
                                print("spawn " .. properties.ent_name .. " at " .. x .. "," .. y)
                            else
                                print("no " .. properties.ent_name .. " class exists")
                            end
                        end
                    end
                end
            end
        end

        -- LocalGame.world:add(WorkingMan(50, -50))
    end

    ns.gamelogic.fini = function()
    end

    ns.gamelogic.OnPreUpdate = function()
        LocalGame.win_size = neko.window_size()

        if ns.edit.get_enabled() then
            return
        end
        LocalGame.mouse_pos = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())

        game_tick = game_tick + 1

        if game_tick % 400 == 1 then
            -- local text = "生成怪物 ["
            -- random_spawn_npc(8, function(v)
            --     text = text .. "{" .. v.x .. "," .. v.y .. "}"
            -- end)
            -- text = text .. "]"
            -- print(text)
            random_spawn_npc()
            print("生成怪物")
        end

        if game_tick % 40 == 1 then
            ns["tiled"].map_edit(LocalGame.level_1, 0, 10, 10, choose({134, 135}))
        end

    end
    ns.gamelogic.OnUpdate = function()
        local dt = neko.dt()

        draw_fixtures = ns.edit.get_enabled()

        if not ns.edit.get_enabled() then

            LocalGame.b2:step(dt)
            LocalGame.world:update(dt)

            cursor:update(dt)
        end
    end

    ns.gamelogic.OnDraw = function()
        local dt = neko.dt()

        LocalGame.world:draw()

        if not ns.edit.get_enabled() then
            cursor:draw()
        end
    end

    ns.gamelogic.OnDrawUI = function()
        local dt = neko.dt()
        -- default_font:draw(("fps: %.2f (%.4f) 分数: %d"):format(1 / dt, dt * 1000, LocalGame.score), 300, 0, 24)

        default_font:draw(("分数: %d 血量: %d"):format(LocalGame.score, player.hp), 40, LocalGame.win_size.y - 60, 36)
    end

    -- neko.before_quit = function()
    --     print("before_quit")
    -- end

    print("haha.lua loaded")
end

function haha_test_gui()
    win = ng.add {
        gui_window = {
            title = 'parent window!'
        },
        gui = {
            valign = ng.GA_TABLE,
            halign = ng.GA_MAX
        }
    }

    text1 = ng.add {
        transform = {
            parent = ns.gui_window.get_body(win)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        },
        gui_text = {
            str = 'body is a very very long text\nof many lines!'
        }
    }

    win2 = ng.add {
        gui_window = {
            title = 'child window!'
        },
        gui_rect = {
            hfill = true
        },
        transform = {
            parent = ns.gui_window.get_body(win)
        },
        gui = {
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        }
    }

    text2 = ng.add {
        transform = {
            parent = ns.gui_window.get_body(win2)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        },
        gui_text = {
            str = 'prop1: x, y, z'
        }
    }

    win = ng.add {
        gui_window = {
            title = 'parent window 2!',
            closeable = false
        },
        gui = {
            valign = ng.GA_TABLE,
            halign = ng.GA_MAX
        }
    }

    win2 = ng.add {
        gui_window = {
            title = 'child window!'
        },
        gui_rect = {
            hfill = true
        },
        transform = {
            parent = ns.gui_window.get_body(win)
        },
        gui = {
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        }
    }

    text2 = ng.add {
        transform = {
            parent = ns.gui_window.get_body(win2)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        },
        gui_text = {
            str = 'body is a very very long text\nof many lines!'
        }
    }

    win = ng.add {
        transform = {
            position = ng.Vec2(42, -42)
        },
        gui_window = {
            title = 'free window!'
        }
    }

    text1 = ng.add {
        transform = {
            parent = ns.gui_window.get_body(win)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        },
        gui_text = {
            str = 'drag titlebar to move'
        }
    }

    win2 = ng.add {
        gui_window = {
            title = 'child window!'
        },
        gui_rect = {
            hfill = true
        },
        transform = {
            parent = ns.gui_window.get_body(win)
        },
        gui = {
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        }
    }

    text2 = ng.add {
        transform = {
            parent = ns.gui_window.get_body(win2)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        },
        gui_text = {
            str = 'I move with my parent!'
        }
    }

    textedit = ng.add {
        transform = {
            parent = ns.gui_window.get_body(win2)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        },
        gui_text = {
            str = 'edit me!!'
        },
        gui_event = {
            key_down = function(e, k)
                if k == ng.KC_A then
                    print('A!')
                end
            end
        },
        gui_textedit = {}
    }

    textedit = ng.add {
        transform = {
            parent = ns.gui_window.get_body(win2)
        },
        gui = {
            color = ng.color_white,
            valign = ng.GA_TABLE,
            halign = ng.GA_MIN
        },
        gui_text = {
            str = "I'm editable too..."
        },
        gui_textedit = {}
    }
end

function haha_huge()
    hot_require 'oscillator'
    hot_require 'rotator'

    ns.sprite.set_atlas('assets/atlas.ase')

    -- add camera: 32 pixels is one unit

    camera = ns.entity.create()
    ns.transform.add(camera)
    ns.camera.add(camera)
    ns.camera.set_viewport_height(camera, 18)

    -- add some blocks

    math.randomseed(os.time())

    function symrand()
        return 2 * math.random() - 1
    end

    local n_blocks = ng.args[2] or 3000
    print('creating ' .. n_blocks .. ' blocks')
    for i = 0, n_blocks do
        local block = ns.entity.create()

        local y = 8 * symrand()
        while math.abs(y) < 1.5 do
            y = 8 * symrand()
        end
        local pos = ng.Vec2(8 * symrand(), y)

        ns.transform.add(block)
        ns.transform.set_position(block, pos)

        ns.sprite.add(block)
        if symrand() < 0 then
            ns.sprite.set_texcell(block, ng.Vec2(0.0, 32.0))
            -- ns.edit.select[block] = true
        else
            ns.sprite.set_texcell(block, ng.Vec2(32.0, 32.0))
        end
        ns.sprite.set_texsize(block, ng.Vec2(32.0, 32.0))

        ns.oscillator.add(block, {
            amp = 3 * math.random(),
            freq = math.random()
        })
        ns.rotator.add(block, math.random() * math.pi)
    end

    -- add player

    player = ns.entity.create()

    ns.transform.add(player)
    ns.transform.set_position(player, ng.Vec2(0.0, 0.0))

    ns.sprite.add(player)
    ns.sprite.set_texcell(player, ng.Vec2(0.0, 32.0))
    ns.sprite.set_texsize(player, ng.Vec2(32.0, 32.0))

    ns.transform.set_scale(player, ng.Vec2(2, 2))
end

function haha_basic()
    hot_require 'oscillator'
    hot_require 'rotator'

    ns.sprite.set_atlas('assets/atlas.ase')

    -- add camera: 32 pixels is one unit

    print("basic.lua loading")

    camera = ns.entity.create()

    ns.transform.add(camera)
    ns.camera.add(camera)
    ns.camera.set_viewport_height(camera, 18)

    -- add some blocks

    local n_blocks = 20
    for i = 0, n_blocks do
        local block = ng.add {
            transform = {
                position = ng.Vec2(i - 8, i - 8)
            },
            sprite = {
                texcell = ng.Vec2(i / 2 == math.floor(i / 2) and 0 or 32, 32),
                texsize = ng.Vec2(32, 32)
            }
        }

        ns.oscillator.add(block, {
            amp = 1,
            freq = 1,
            phase = 5 * i / n_blocks
        })
        ns.rotator.add(block, 2 * math.pi)
    end

    -- add player

    player = ng.add {
        transform = {
            position = ng.Vec2(0, 0),
            scale = ng.Vec2(2, 2),
            rotation = math.pi / 16
        },
        sprite = {
            texcell = ng.Vec2(0, 32),
            texsize = ng.Vec2(32, 32)
        }
    }

    rchild = ng.add {
        transform = {
            parent = player,
            position = ng.Vec2(1, 0),
            scale = ng.Vec2(0.5, 0.5)
        },
        sprite = {
            texcell = ng.Vec2(32, 32),
            texsize = ng.Vec2(32, 32)
        }
    }

    lchild = ng.add {
        transform = {
            parent = player,
            position = ng.Vec2(-1, 0),
            scale = ng.Vec2(0.5, 0.5)
        },
        sprite = {
            texcell = ng.Vec2(32, 32),
            texsize = ng.Vec2(32, 32)
        }
    }

    -- local tiletype, layers = TiledMap_Parse("assets/maps/map.tmx")

    -- print(type(tiletype))

    -- local pink = hot_require("libs/ink/ink")
    -- local story = pink('game.ink')

    -- while true do
    --     -- 2) Game content, line by line
    --     while story.canContinue do
    --         print(story.continue())
    --     end
    --     -- 3) Display story.currentChoices list, allow player to choose one
    --     if #story.currentChoices == 0 then
    --         break
    --     end -- cannot continue and there are no choices
    --     for i = 1, #story.currentChoices do
    --         print(i .. "> " .. story.currentChoices[i].text)
    --     end

    --     local answer = io.read("*number")
    --     story.chooseChoiceIndex(tonumber(answer))
    -- end

    -- entity destruction

    print("basic.lua loaded")
end

function test_ecs(dt)
    ecs:update()

    for id, e in ecs:select{"pos", "vel"} do
        e.pos = e.pos + e.vel * vec2(dt, dt)

        if neko.key_down "p" then
            e.vel = nil
        end

        if neko.key_down "k" then
            ecs:kill(id)
        end
    end

    for id, e in ecs:select{"pos", "follow"} do
        local other = ecs:get(e.follow)
        if other ~= nil then
            e.pos = e.pos:lerp(other.pos, dt)
        end
    end

    for id, e in ecs:select{"pos", "rot"} do
        e.rot.angle = e.rot.angle + e.rot.delta * dt
    end

    for id, e in ecs:query{
        select = {"pos", "img", "z_index"},
        where = function(e)
            return e.pos.x < neko.window_width() / 2
        end,
        order_by = function(lhs, rhs)
            return lhs.entity.z_index < rhs.entity.z_index
        end
    } do
        local ox = e.img:width() * 0.5
        local oy = e.img:height() * 0.5
        local angle = e.rot and e.rot.angle or 0
        local scale = e.scale or 1

        e.img:draw(e.pos.x, e.pos.y, angle, scale * 3, scale * 3, ox, oy)

        if e.vel == nil and neko.key_down "v" then
            e.vel = vec2(4, 1)
        end
    end
end

local function load_world()
    local b2 = neko.b2_world {
        gx = 0,
        gy = 0,
        meter = 16
    }
    local world = World()

    local tilemap = neko.tilemap_load "assets/map.ldtk"
    tilemap:make_collision(b2, "Collision", {1})

    for k, v in ipairs(tilemap:entities()) do
        local mt
        if v.id == "Skeleton" then
            mt = _G["Npc"]
        else
            mt = _G[v.id]
        end

        if mt ~= nil then
            local obj = world:add(mt(v.x, v.y, v.id))
            if v.id == "CPlayer" then
                player = obj
            end
            -- print(v.id, v.x, v.y)
        else
            print("no " .. v.id .. " class exists")
        end
    end
    return world, b2, tilemap
end

-- ffi.cdef [[
--     typedef struct {
--         float x;
--         float y;
--         float z;
--         float w;
--     } neko_vec4_t;
--     typedef struct {
--         uint32_t id;
--     } __lua_tex_t;
--     typedef struct {
--         float v[16];
--     } __lua_quad_vdata_t;
--     typedef struct {
--         uint32_t v[6];
--     } __lua_quad_idata_t;
--     ]]

--[[
    function neko.__define_default_callbacks()


    local ffi = FFI


        local TEX_WIDTH = 128
        local TEX_HEIGHT = 128

        local gd = {}



        neko.game_init = function()
            print("neko.game_init")


            local win = ng.game_get_window_size()
            local win_w, win_h = win.x, win.y

            gd.main_fbo = neko.render_framebuffer_create()
            gd.main_rt = neko.render_texture_create(win_w, win_h, {
                type = "R_TEXTURE_2D",
                format = "R_TEXTURE_FORMAT_RGBA32F",
                wrap_s = "R_TEXTURE_WRAP_REPEAT",
                wrap_t = "R_TEXTURE_WRAP_REPEAT",
                min_filter = "R_TEXTURE_FILTER_NEAREST",
                mag_filter = "R_TEXTURE_FILTER_NEAREST"
            })
            gd.main_rp = neko.render_renderpass_create(gd.main_fbo, gd.main_rt)

            gd.fbo = neko.render_framebuffer_create()
            gd.rt = neko.render_texture_create(win_w, win_h)
            gd.rp = neko.render_renderpass_create(gd.fbo, gd.rt)

            gd.rp_default = neko.render_renderpass_default()

            -- gd.fontbatch = neko.fontbatch_create(font_vs, font_ps)

            -- test_shader = neko.render_shader_create("compute", {
            --     -- VERTEX = sprite_vs,
            --     -- FRAGMENT = sprite_fs
            --     COMPUTE = comp_src
            -- })

            test_uniform = neko.render_uniform_create("u_roll", {{
                type = "R_UNIFORM_FLOAT"
            }})

            test_tex = neko.render_texture_create(TEX_WIDTH, TEX_HEIGHT, {
                type = "R_TEXTURE_2D",
                format = "R_TEXTURE_FORMAT_RGBA32F",
                wrap_s = "R_TEXTURE_WRAP_REPEAT",
                wrap_t = "R_TEXTURE_WRAP_REPEAT",
                min_filter = "R_TEXTURE_FILTER_NEAREST",
                mag_filter = "R_TEXTURE_FILTER_NEAREST"
            })

            -- test_pipeline = neko.render_pipeline_create("test_pipeline", {
            --     compute = {
            --         shader = test_shader
            --     }
            -- })

            local test_vec4_data = ffi.new("neko_vec4_t")

            test_vec4_data.x = 0.0
            test_vec4_data.y = 1.0
            test_vec4_data.z = 1.0
            test_vec4_data.w = 1.0

            -- test_storage_buffer = neko.render_storage_buffer_create("u_voxels", test_vec4_data,
            --     ffi.sizeof("neko_vec4_t"))

            -- test_custom_sprite = {}
            -- test_custom_sprite.tex = neko.render_texture_create(10, 10, {
            --     type = "R_TEXTURE_2D",
            --     data = neko.gen_tex(),
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

            -- test_custom_sprite.vbo = neko.render_vertex_buffer_create("vbo", test_custom_sprite.vdata,
            --     ffi.sizeof("__lua_quad_vdata_t"))
            -- test_custom_sprite.ibo = neko.render_index_buffer_create("ibo", test_custom_sprite.idata,
            --     ffi.sizeof("__lua_quad_idata_t"))

            -- test_custom_sprite.uniform = neko.render_uniform_create("u_tex", {{
            --     type = "R_UNIFORM_SAMPLER2D"
            -- }}, "R_SHADER_STAGE_FRAGMENT")

            -- test_custom_sprite.shader = neko.render_shader_create("quad", {
            --     VERTEX = custom_sprite_vs,
            --     FRAGMENT = custom_sprite_fs
            -- })

            -- test_custom_sprite.v_attr, test_custom_sprite.v_attr_size =
            --     neko.render_vertex_attribute_create("vertex_attribute_name", {{
            --         name = "a_pos",
            --         format = "R_VERTEX_ATTRIBUTE_FLOAT2"
            --     }, {
            --         name = "a_uv",
            --         format = "R_VERTEX_ATTRIBUTE_FLOAT2"
            --     }})

            -- test_custom_sprite.pipeline = neko.render_pipeline_create("test_pipeline", {
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

            neko.render_renderpass_fini(gd.rp)
            neko.render_texture_fini(gd.rt)
            neko.render_framebuffer_fini(gd.fbo)

            neko.render_renderpass_fini(gd.main_rp)
            neko.render_texture_fini(gd.main_rt)
            neko.render_framebuffer_fini(gd.main_fbo)
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

            local win_w, win_h = neko.render_display_size()

            -- mm({win_w, win_h})

            local roll = ns.timing.get_elapsed() * 0.001

            -- neko.render_pipeline_bind(test_pipeline)
            -- neko.render_apply_bindings({
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
            -- neko.render_dispatch_compute(TEX_WIDTH / 16, TEX_HEIGHT / 16, 1)

            -- local v1 = to_vec2(0.0, 0.0)
            -- local v2 = to_vec2(TEX_WIDTH, TEX_HEIGHT)

            -- neko.idraw_defaults()
            -- neko.idraw_camera2d(fbs_x, fbs_y)

            -- neko.idraw_texture(test_tex)
            -- neko.idraw_rectvd(v1, v2, to_vec2(0.0, 0.0), to_vec2(1.0, 1.0), "R_PRIMITIVE_TRIANGLES",
            --     to_color(255, 255, 255, 255))

            -- neko.render_renderpass_begin(gd.main_rp)
            -- neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
            -- neko.render_clear(0.0, 0.0, 0.0, 0.0)
            -- neko.idraw_draw()
            -- neko.render_renderpass_end()

            -- neko.render_renderpass_begin(gd.main_rp)
            -- neko.render_pipeline_bind(test_custom_sprite.pipeline)
            -- neko.render_apply_bindings({
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
            -- neko.render_draw({
            --     start = 0,
            --     count = 6
            -- })
            -- neko.render_renderpass_end()

            neko.idraw_defaults()
            neko.idraw_camera3d(fbs_x, fbs_y)
            neko.idraw_translatef(0.0, 0.0, -2.0)
            neko.idraw_rotatev(ns.timing.get_elapsed() * 0.0001, 1.0, 0.0, 0.0)
            neko.idraw_rotatev(ns.timing.get_elapsed() * 0.0001, 0.0, 1.0, 0.0)
            neko.idraw_rotatev(ns.timing.get_elapsed() * 0.0001, 0.0, 0.0, 1.0)
            neko.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 200, 100, 50, 255, "R_PRIMITIVE_LINES")

            neko.render_renderpass_begin(gd.rp)
            neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
            neko.render_clear(0.0, 0.0, 0.0, 1.0)
            neko.idraw_draw()
            neko.render_renderpass_end()

            neko.idraw_camera3d(fbs_x, fbs_y)
            neko.idraw_depth_enabled(true)
            neko.idraw_face_cull_enabled(true)
            neko.idraw_translatef(0.0, 0.0, -1.0)
            neko.idraw_texture(gd.rt)
            neko.idraw_rotatev(ns.timing.get_elapsed() * 0.0001, 1.0, 0.0, 0.0)
            neko.idraw_rotatev(ns.timing.get_elapsed() * 0.0002, 0.0, 1.0, 0.0)
            neko.idraw_rotatev(ns.timing.get_elapsed() * 0.0003, 0.0, 0.0, 1.0)
            neko.idraw_box(0.0, 0.0, 0.0, 0.5, 0.5, 0.5, 255, 255, 255, 255, "R_PRIMITIVE_TRIANGLES")

            neko.render_renderpass_begin(gd.rp_default)
            neko.render_set_viewport(0.0, 0.0, fbs_x, fbs_y)
            neko.idraw_draw()
            neko.render_renderpass_end()

            neko.idraw_defaults()
            neko.idraw_camera2d(fbs_x, fbs_y)
            neko.idraw_texture(gd.main_rt)
            neko.idraw_rectvd(to_vec2(0.0, 0.0), to_vec2(win_w, win_h), to_vec2(0.0, 0.0), to_vec2(1.0, 1.0),
                "R_PRIMITIVE_TRIANGLES", to_color(255, 255, 255, 255))

            neko.render_renderpass_begin(gd.rp_default)
            neko.render_set_viewport(0.0, 0.0, win_w, win_h)
            -- neko.idraw_draw()
            -- neko.render_clear(0.0, 0.0, 0.0, 1.0)
            -- neko.fontbatch_draw(gd.fontbatch)
            neko.render_renderpass_end()

        end

    end

]]
