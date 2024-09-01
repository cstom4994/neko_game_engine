
#begin VERTEX

#version 330 core

layout(location=0) in vec2 a_position;
layout(location=1) in vec2 a_texindex;

out vec2 v_texindex;
uniform mat3 inverse_view_matrix;
uniform float scale;

void main() {
    vec2 position = a_position * 1.0; // 应用缩放因子
    // position.y = -position.y;
    gl_Position = vec4(inverse_view_matrix * vec3(position, 1.0), 1.0);
    v_texindex = a_texindex;
}

#end VERTEX

#begin FRAGMENT

#version 330 core
in vec2 v_texindex;
out vec4 f_color;
uniform sampler2D u_texture;
void main() {
    f_color = texture(u_texture, v_texindex);
}

#end FRAGMENT