
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

uniform int outline_enable = 0;
uniform float outline_thickness = 0.2;
uniform vec3 outline_colour = vec3(1, 1, 1);
uniform float outline_threshold = 0;

uniform int glow_enable = 0;
uniform float glow_size = 0.4;
uniform vec3 glow_colour = vec3(1, 1, 1);
uniform float glow_intensity = 1;
uniform float glow_threshold = 0;

uniform int bloom_enable = 0;
uniform float bloom_spread = 1;
uniform float bloom_intensity = 0.2;

void main() {
    f_color = texture(u_texture, v_texindex);

    if (outline_enable == 1 && f_color.a <= outline_threshold) {
        ivec2 size = textureSize(u_texture, 0);

        float uv_x = v_texindex.x * size.x;
        float uv_y = v_texindex.y * size.y;

        float sum = 0.0;
        for (int n = 0; n < 9; ++n) {
            uv_y = (v_texindex.y * size.y) + (outline_thickness * float(n - 4.5));
            float h_sum = 0.0;
            h_sum += texelFetch(u_texture, ivec2(uv_x - (4.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x - (3.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x - (2.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x - outline_thickness, uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x, uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x + outline_thickness, uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x + (2.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x + (3.0 * outline_thickness), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x + (4.0 * outline_thickness), uv_y), 0).a;
            sum += h_sum / 9.0;
        }

        if (sum / 9.0 >= 0.0001) {
            f_color = vec4(outline_colour, 1);
        }
    }

    if (glow_enable == 1 && f_color.a <= glow_threshold) {
        ivec2 size = textureSize(u_texture, 0);
	
        float uv_x = v_texindex.x * size.x;
        float uv_y = v_texindex.y * size.y;

        float sum = 0.0;
        for (int n = 0; n < 9; ++n) {
            uv_y = (v_texindex.y * size.y) + (glow_size * float(n - 4.5));
            float h_sum = 0.0;
            h_sum += texelFetch(u_texture, ivec2(uv_x - (4.0 * glow_size), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x - (3.0 * glow_size), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x - (2.0 * glow_size), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x - glow_size, uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x, uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x + glow_size, uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x + (2.0 * glow_size), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x + (3.0 * glow_size), uv_y), 0).a;
            h_sum += texelFetch(u_texture, ivec2(uv_x + (4.0 * glow_size), uv_y), 0).a;
            sum += h_sum / 9.0;
        }

        f_color = vec4(glow_colour, (sum / 9.0) * glow_intensity);
    }

    if (bloom_enable == 1) {
        ivec2 size = textureSize(u_texture, 0);

        float uv_x = v_texindex.x * size.x;
        float uv_y = v_texindex.y * size.y;

        vec4 sum = vec4(0.0);
        for (int n = 0; n < 9; ++n) {
            uv_y = (v_texindex.y * size.y) + (bloom_spread * float(n - 4));
            vec4 h_sum = vec4(0.0);
            h_sum += texelFetch(u_texture, ivec2(uv_x - (4.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(u_texture, ivec2(uv_x - (3.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(u_texture, ivec2(uv_x - (2.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(u_texture, ivec2(uv_x - bloom_spread, uv_y), 0);
            h_sum += texelFetch(u_texture, ivec2(uv_x, uv_y), 0);
            h_sum += texelFetch(u_texture, ivec2(uv_x + bloom_spread, uv_y), 0);
            h_sum += texelFetch(u_texture, ivec2(uv_x + (2.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(u_texture, ivec2(uv_x + (3.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(u_texture, ivec2(uv_x + (4.0 * bloom_spread), uv_y), 0);
            sum += h_sum / 9.0;
        }

        f_color = texture(u_texture, v_texindex) - ((sum / 9.0) * bloom_intensity);
    }
}

#end FRAGMENT