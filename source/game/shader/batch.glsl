
#begin VERTEX

#version 330 core

layout(location=0) in vec2 a_position;
layout(location=1) in vec2 a_texindex;

out vec2 v_texindex;
uniform mat3 inverse_view_matrix;
uniform float scale;

void main() {
    vec2 position = a_position * 1.0; // 应用缩放因子
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

uniform int pixelate_enable = 0;
uniform float pixelate_value = 0.0;

uniform int bitrate = 8;

uniform int trans_enable = 0;

vec4 channelBitrate(vec4 incol, int bit)
{
	float bitdepth = pow(2.0, abs(float(bit)));
	vec4 outcol = floor(incol * bitdepth) /bitdepth;
	return outcol;
}

void main() {
    //  f_color = channelBitrate(texture(u_texture, v_texindex),8);

    vec2 uv = v_texindex;

    if (pixelate_enable == 1) {
        // uv = (floor((uv * pixelate_value) / 2.0) * 2.0 + 1.0) / pixelate_value; // 像素化处理

        vec2 pixel_count = vec2(pixelate_value, pixelate_value * (9.0/16.0));

        uv *= pixel_count;
        uv = floor(uv);
        uv = uv / pixel_count;
    }

    f_color = texture(u_texture, uv);

    if (outline_enable == 1 && f_color.a <= outline_threshold) {
        ivec2 size = textureSize(u_texture, 0);

        // 计算当前纹理坐标
        float uv_x = uv.x * size.x;
        float sum = 0.0;

        // 垂直方向的轮廓采样
        for (int n = 0; n < 9; ++n) {
            float uv_y = (uv.y * size.y) + outline_thickness * (float(n) - 4.5);
            float h_sum = 0.0;

            // 水平方向的轮廓采样
            for (int m = -4; m <= 4; ++m) {
                h_sum += texelFetch(u_texture, ivec2(uv_x + float(m) * outline_thickness, uv_y), 0).a;
            }
            sum += h_sum / 9.0;
        }

        // 判断是否应用轮廓颜色
        if (sum / 9.0 >= 0.0001) {
            f_color = vec4(outline_colour, 1);
        }
    }

    if (glow_enable == 1 && f_color.a <= glow_threshold) {
        ivec2 size = textureSize(u_texture, 0);
	
        float uv_x = uv.x * size.x;
        float uv_y = uv.y * size.y;

        float sum = 0.0;
        for (int n = 0; n < 9; ++n) {
            uv_y = (uv.y * size.y) + (glow_size * float(n - 4.5));
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

        float uv_x = uv.x * size.x;
        float uv_y = uv.y * size.y;

        vec4 sum = vec4(0.0);
        for (int n = 0; n < 9; ++n) {
            uv_y = (uv.y * size.y) + (bloom_spread * float(n - 4));
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

        f_color = texture(u_texture, uv) - ((sum / 9.0) * bloom_intensity);
    }

    if(trans_enable == 1)
    {
        f_color = vec4(f_color.rgb, f_color.a / 2);
    }
}

#end FRAGMENT