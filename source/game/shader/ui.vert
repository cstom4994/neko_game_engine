#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float mode;

uniform mat4 camera = mat4(1.0);

out VS_OUT {
	vec2 uv;
	vec4 color;
	float mode;
} vs_out;

void main() {
	vs_out.uv = uv;
	vs_out.color = color;
	vs_out.mode = mode;

	gl_Position = camera * vec4(position, 0.0, 1.0);
}
