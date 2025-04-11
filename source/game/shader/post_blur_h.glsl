#begin VERTEX

#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 position;
out vec2 tex_coord;
out vec2 v_blur_tex_coords[11];

uniform vec2 NekoScreenSize;

void main() {
    gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
    tex_coord = a_uv;
    position = a_position;
    vec2 center_tex_coords = a_position * 0.5 + 0.5;
    float pixel_size = 1.0 / NekoScreenSize.x;
    for (int i = -5; i <= 5; ++i) {
        v_blur_tex_coords[i + 5] = center_tex_coords + vec2(pixel_size * float(i), 0.0);
    }
}

#end VERTEX

#begin FRAGMENT

#version 330 core

uniform vec2 NekoScreenSize;
uniform sampler2D NekoTextureInput;

in vec2 position;
in vec2 tex_coord;
in vec2 v_blur_tex_coords[11];

out vec4 frag_color;

void main() {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0);
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[0]) * 0.0093;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[1]) * 0.028002;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[2]) * 0.065984;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[3]) * 0.121703;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[4]) * 0.175713;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[5]) * 0.198596;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[6]) * 0.175713;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[7]) * 0.121703;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[8]) * 0.065984;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[9]) * 0.028002;
    frag_color += texture(NekoTextureInput, v_blur_tex_coords[10]) * 0.0093;
}


#end FRAGMENT
