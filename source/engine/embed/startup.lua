-- Copyright(c) 2022-2023, KaoruXun All rights reserved.
-- runs on start of the engine
-- used to load scripts
neko = require "neko.core"
ffi = require("neko.ffi")
cffi = require("neko.cffi")
luastruct_test = require("neko.struct_test")
pack = require("neko.pack")
luadb = require "neko.luadb"

dump_func = function(tbl, indent)
    if not indent then
        indent = 0
        print("inspect: \"" .. tostring(tbl) .. "\"")
    end
    for k, v in pairs(tbl) do
        formatting = string.rep("  ", indent) .. k .. ": "
        if type(v) == "table" then
            print(formatting)
            dump_func(v, indent + 1)
        elseif type(v) == 'boolean' then
            print(formatting .. tostring(v))
        else
            print(formatting .. v)
        end
    end
end

unsafe_require = require
print_ = print
print = neko.print

if os.getenv "LOCAL_LUA_DEBUGGER_VSCODE" == "1" then
    unsafe_require("lldebugger").start()
    print("LOCAL_LUA_DEBUGGER_VSCODE=1")
end

common = require "common"

neko_file_path = common.memoize(function(path)
    return __neko_file_path(path)
end)

function starts_with(str, start)
    return str:sub(1, #start) == start
end

function ends_with(str, ending)
    return ending == "" or str:sub(-#ending) == ending
end

function sleep(n)
    local t0 = os.clock()
    while os.clock() - t0 <= n do
    end
end

function to_vec2(_x, _y)
    return {
        x = _x,
        y = _y
    }
end

function to_color(r, g, b, a)
    return {
        r = r,
        g = g,
        b = b,
        a = a
    }
end

function easeOutCubic(x)
    return 1 - math.pow(1 - x, 3)
end

function read_file(filename)
    local file = io.open(filename, "r") -- 打开文件只读模式
    if not file then
        return nil
    end -- 文件不存在或者无法打开时返回nil

    local content = file:read("*all") -- 读取文件内容
    file:close() -- 关闭文件

    return content -- 返回文件内容
end

function table_merge(t1, t2)
    for k, v in pairs(t2) do
        if type(v) == "table" then
            if type(t1[k] or false) == "table" then
                table_merge(t1[k] or {}, t2[k] or {})
            else
                t1[k] = v
            end
        else
            t1[k] = v
        end
    end
    return t1
end

function neko_ls(path)
    local result = {}
    local tb = __neko_ls(path)
    for _, tb1 in pairs(tb) do
        if tb1["isDirectory"] == true then
            result = table_merge(result, neko_ls(tb1["path"]))
        else
            table.insert(result, tb1["path"])
        end
    end
    return result
end

__NEKO_CONFIG_TYPE_INT = 0
__NEKO_CONFIG_TYPE_FLOAT = 1
__NEKO_CONFIG_TYPE_STRING = 2

sprite_vs = [[
#version 330

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float use_texture;

uniform mat4 tiled_sprite_camera;

out VS_OUT {
	vec4 color;
	vec2 uv;
	float use_texture;
} vs_out;

void main() {
	vs_out.color = color;
	vs_out.uv = uv;
	vs_out.use_texture = use_texture;

	gl_Position = tiled_sprite_camera * vec4(position, 0.0, 1.0);
}
]]

sprite_fs = [[
#version 330

out vec4 color;

in VS_OUT {
    vec4 color;
    vec2 uv;
    float use_texture;
} fs_in;

uniform sampler2D batch_texture;

void main() {
    vec4 texture_color = vec4(1.0);

    if (fs_in.use_texture == 1.0) {
        texture_color = texture(batch_texture,  fs_in.uv);
    }

    color = fs_in.color * texture_color;
}
]]

batch_vs = [[
#version 330

uniform mat4 u_mvp;
in vec2 in_pos; in vec2 in_uv;
out vec2 v_uv;

void main() {
    v_uv = in_uv;
    gl_Position = u_mvp * vec4(in_pos, 0, 1);
}
]]

batch_ps = [[
#version 330

precision mediump float;
uniform sampler2D u_sprite_texture;
in vec2 v_uv; out vec4 out_col;

void main() { out_col = texture(u_sprite_texture, v_uv); }
]]

comp_src = [[
#version 430 core
uniform float u_roll;
layout(rgba32f, binding = 0) uniform image2D destTex;
layout (std430, binding = 1) readonly buffer u_voxels {
    vec4 data;
};
layout (local_size_x = 16, local_size_y = 16) in;
void main() {
ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy) - 8 ) / 8.0);
float globalCoef = sin(float(gl_WorkGroupID.x + gl_WorkGroupID.y) * 0.1 + u_roll) * 0.5;
vec4 rc = vec4(1.0 - globalCoef * localCoef, globalCoef * localCoef, 0.0, 1.0);
vec4 color = mix(rc, data, 0.5f);
imageStore(destTex, storePos, color);
}
]]

custom_sprite_vs = [[
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
precision mediump float;
out vec2 uv;
void main()
{
   gl_Position = vec4(a_pos, 0.0, 1.0);
   uv = a_uv;
}
]]

custom_sprite_fs = [[
#version 330 core
precision mediump float;
uniform sampler2D u_tex;
in vec2 uv;
out vec4 frag_color;
void main()
{
   frag_color = texture(u_tex, uv);
}
]]

print("lua startup")
