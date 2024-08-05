-- 添加nekogame需要路径并加载nekogame
package.path = package.path .. ';' .. nekogame_script_path .. 'script/?.lua'
package.path = package.path .. ';' .. nekogame_script_path .. 'script/?/init.lua'
package.path = package.path .. ';' .. nekogame_script_path .. 'script/libs/?.lua'
package.cpath = package.cpath .. ";" .. nekogame_script_path .. "script/libs/?.dll"

require 'nekogame'

-- misc
ns.app = {}
function ns.app.key_down(key)
    if ns.gui.has_focus() then
        return
    end

    if key == ng.KC_Q then
        ns.game.quit()

    elseif key == ng.KC_1 then
        ns.console.set_visible(not ns.console.get_visible())

    elseif key == ng.KC_E then
        ns.timing.set_paused(not ns.edit.get_enabled())
        ns.edit.set_enabled(not ns.edit.get_enabled())

    elseif key == ng.KC_C then
        print("destroying group 'default'")
        ns.group.destroy('default')

    end
end

local run = (ng.args[1] and loadfile(ng.args[1])) or loadfile('./main.lua')
if run then
    -- 运行给定的脚本
    run()

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

    require('test').UnitTest()

    -- 进入编辑模式
    ns.timing.set_paused(true)
    ns.edit.set_enabled(true)
end

luainspector = Inspector.inspector_init()

game_imgui = require("cimgui")

ns.edit.undo_save()

print("main.lua loaded")
