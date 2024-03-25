-- Copyright(c) 2022-2023, KaoruXun All rights reserved.
-- runs on start of the engine
-- used to load scripts
dump_func = require "common/dump"

ffi = require("cffi")

unsafe_require = require
print = neko.print

if os.getenv "LOCAL_LUA_DEBUGGER_VSCODE" == "1" then
    unsafe_require("lldebugger").start()
    print("LOCAL_LUA_DEBUGGER_VSCODE=1")
end

common = require "common/common"

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

function to_vec2(x, y)
    return {
        x = x,
        y = y
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

__NEKO_CONFIG_TYPE_INT = 0
__NEKO_CONFIG_TYPE_FLOAT = 1
__NEKO_CONFIG_TYPE_STRING = 2

sprite_vs = [[
#version 430

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
#version 430

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
