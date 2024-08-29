
#begin VERTEX

#version 330 core

layout(location=0) in vec2 a_position;
layout(location=1) in vec2 a_texindex;

out vec2 v_texindex;
uniform mat3 inverse_view_matrix;
uniform float scale;

void main() {
    float adjusted_scale = scale;
    if (scale == 0.0) {
        adjusted_scale = 0.1;
    }
    gl_Position = vec4(inverse_view_matrix * adjusted_scale * vec3(a_position, 1.0), 1.0);
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