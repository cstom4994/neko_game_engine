#version 330 core
layout(location=0) in vec2 a_position;
layout(location=1) in vec2 a_texindex;
out vec2 v_texindex;
// uniform mat4 u_mvp;
uniform mat3 inverse_view_matrix;
void main() {
    // gl_Position = u_mvp * vec4(a_position, 0.0, 1.0);
    gl_Position = vec4(inverse_view_matrix * vec3(a_position, 1.0), 1.0);
    v_texindex = a_texindex;
}