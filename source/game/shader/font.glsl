
#begin VERTEX

#version 330 core
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoord;

out vec2 TexCoord;

// uniform mat4 projection;

uniform mat3 inverse_view_matrix;

void main() {

	vec3 transformed_position = inverse_view_matrix * 0.05 * vec3(inPos, 1.0);
    transformed_position.y = -transformed_position.y;  // 翻转Y轴

    gl_Position = vec4(transformed_position, 1.0);
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