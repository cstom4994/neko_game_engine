-- 添加nekogame需要路径并加载nekogame
-- hot_require 'nekogame'
mu = neko.ui
db = neko.db
imgui = neko.imgui

function neko.__define_default_callbacks()
    neko.before_quit = function()
    end
    neko.args = function(args)
        neko.__start(args)
    end
end

-- misc
ns.app = {}
function ns.app.OnKeyDown(key)
    if key == "KC_F1" then
        ns.timing.set_paused(not ns.edit.get_enabled())
        ns.edit.set_enabled(not ns.edit.get_enabled())
        neko.show_mouse(ns.edit.get_enabled())
    end

    if neko.imgui.IsCapturedEvent() then
        return
    end

    local keyname = key

    if keyname == "KC_F2" then
        ns.inspector.set_visible(not ns.inspector.get_visible())
    end

    if ns.edit.get_enabled() then
        if keyname == "KC_C" then

        end
    end
end

local CheckDefaultCamera = function()
    -- 没有相机则添加默认值
    if ng.is_nil_entity(ns.camera.get_current_camera()) then
        ng.add {
            transform = {
                position = ng.Vec2(0, 0)
            },
            camera = {
                viewport_height = 18.75
            }
        }
        print("Add camera")
    else
        print("Default camera")
    end
end

local function errorHandler(err)
    __print("Error: ", err)
end

local run, err = (ng.args[1] and loadfile(ng.args[1])) or loadfile('./main.lua') or haha
if run then

    -- 运行给定的脚本
    local status, result = xpcall(run, errorHandler)
    if status then
        print("run successful")
    else
        print("run failed")
    end

    CheckDefaultCamera()
else
    -- 没有启动脚本

    CheckDefaultCamera()

    -- neko.set_window_title("Sandbox")

    if ng.args[1] == "unittest" then
        hot_require('test').UnitTest()
    else
        print("No script to run")
    end

    -- 进入编辑模式
    ns.timing.set_paused(true)
    ns.edit.set_enabled(true)
end

print("nekomain.lua loaded default")
