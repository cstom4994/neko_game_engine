
#begin VERTEX

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
    {	
        vs_out.color = color;
        vs_out.uv = uv;
        vs_out.use_texture = use_texture;
    }
    vec2 pos = position * 1.0; // 应用缩放因子
    pos.y = -pos.y;
    pos = pos;
	vec3 transformed_position = inverse_view_matrix * vec3(pos, 1.0);
    gl_Position = vec4(transformed_position, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 330 core

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

#end FRAGMENT
