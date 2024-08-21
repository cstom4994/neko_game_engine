-- 添加nekogame需要路径并加载nekogame
-- hot_require 'nekogame'
mm = hot_require("libs/mm")

-- luainspector = Inspector.inspector_init()

mu = neko.ui

function neko.__define_default_callbacks()
    neko.game_init_thread = function()
    end
    neko.game_init = function()
    end
    neko.game_fini = function()
    end
    neko.game_pre_update = function()
    end
    neko.game_loop = function(dt)
    end
    neko.game_ui = function()
    end
    neko.game_render = function()
    end
end

-- misc
ns.app = {}
function ns.app.key_down(key)
    if ns.gui.has_focus() then
        return
    end

    if key == ng.KC_1 then
        ns.console.set_visible(not ns.console.get_visible())
    elseif key == ng.KC_E then
        ns.timing.set_paused(not ns.edit.get_enabled())
        ns.edit.set_enabled(not ns.edit.get_enabled())
    elseif key == ng.KC_C then
        print("destroying group 'default'")
        ns.group.destroy('default')
    end
end

local run, err = (ng.args[1] and loadfile(ng.args[1])) or loadfile('./main.lua')
if run then

    local function errorHandler(err)
        __print("Error: ", err)
    end

    -- 运行给定的脚本
    local status, result = xpcall(run, errorHandler)
    if status then
        print("run successful")
    else
        print("run failed")
    end

    -- 没有相机则添加默认值
    if ns.camera.get_current_camera() == ng.entity_nil then
        ng.add {
            camera = {
                viewport_height = 18.75
            }
        }
    end
else
    -- 没有启动脚本

    -- 添加默认相机
    ng.add {
        camera = {
            viewport_height = 18.75
        }
    }

    -- ng.api.set_window_title("Sandbox")

    if ng.args[1] == "unittest" then
        hot_require('test').UnitTest()
    end

    -- 进入编辑模式
    ns.timing.set_paused(true)
    ns.edit.set_enabled(true)
end

ns.edit.undo_save()

print("nekomain.lua loaded default")
