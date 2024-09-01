
#begin VERTEX

#version 330 core
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoord;

out vec2 TexCoord;

uniform mat3 inverse_view_matrix;
uniform mat4 u_mvp;
uniform int mode;

void main() {
    if (mode == 0) {
        vec2 position = inPos * 1.0; // 应用缩放因子
        position.y = -position.y;
        vec3 transformed_position = inverse_view_matrix * vec3(position, 1.0);
        gl_Position = vec4(transformed_position.xy, 0.0, 1.0);
    } else {
        vec2 position = inPos * 1.0; // 应用缩放因子
        vec4 transformed_position = u_mvp * vec4(position, 1.0, 1.0);
        gl_Position = vec4(transformed_position.xy, 0.0, 1.0);
    }
    TexCoord = inTexCoord;
}

#end VERTEX

#begin FRAGMENT

#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoord).a);
    FragColor = vec4(textColor, 1.0) * sampled;
}

#end FRAGMENT