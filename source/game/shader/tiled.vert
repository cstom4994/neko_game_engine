#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float use_texture;

uniform mat3 inverse_view_matrix;

out VS_OUT {
	vec4 color;
	vec2 uv;
	float use_texture;
} vs_out;

void main() {

	vs_out.color = color;
	vs_out.uv = uv;
	vs_out.use_texture = use_texture;

	// gl_Position = vec4(inverse_view_matrix * vec3(position, 1.0), 1.0);

	vec3 transformed_position = inverse_view_matrix * 0.04 * vec3(position, 1.0);
    transformed_position.y = -transformed_position.y;  // 翻转Y轴

    gl_Position = vec4(transformed_position, 1.0);
}