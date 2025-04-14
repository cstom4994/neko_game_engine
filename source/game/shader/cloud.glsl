#begin VERTEX

#version 330 core

layout(location=0) in vec2 a_position;
layout(location=1) in vec2 a_texindex;

out vec2 v_texindex;
uniform mat3 inverse_view_matrix;

void main() {
    vec2 position = a_position * 1.0; // 应用缩放因子
    gl_Position = vec4(inverse_view_matrix * vec3(position, 1.0), 1.0);
    v_texindex = a_texindex;
}

#end VERTEX

#begin FRAGMENT

#version 330 core

in vec2 v_texindex;

uniform float u_time;
uniform vec3 light_pos = vec3(0.0, 0.0, 0.0);

uniform mat3 inverse_view_matrix;
uniform vec3 u_groundColor = vec3(0.1, 0.1, 0.1);

out vec4 FragColor;

vec2 rand_vec(uvec2 p) {

    uint x = p.x;
    uint y = p.y;

    x *= 0x3504f335u;
    y *= 0x8fc1ecd5u;
    x ^= y;
    x *= 741103597u;

    float hash_to_float = float(x >> 8u) * (1.0 / 16777216.0);
    float angle = hash_to_float * radians(360.0);
    return vec2(cos(angle), sin(angle));
}

// https://www.shadertoy.com/view/3tlBzn
// Pixelated drifting 2D cloud made from multiple layers of perlin noise
float perlin_noise(in vec2 point, vec2 wrap, float seed) {
    vec2 point_i = floor(point);
    vec2 point_f = fract(point);

    vec2 vec_tl = rand_vec(uvec2(mod(point_i + vec2(0.0, 0.0), wrap) + uvec2(seed)));
    vec2 vec_tr = rand_vec(uvec2(mod(point_i + vec2(1.0, 0.0), wrap) + uvec2(seed)));
    vec2 vec_bl = rand_vec(uvec2(mod(point_i + vec2(0.0, 1.0), wrap) + uvec2(seed)));
    vec2 vec_br = rand_vec(uvec2(mod(point_i + vec2(1.0, 1.0), wrap) + uvec2(seed)));

    vec2 u = point_f * point_f * (3.0 - 2.0 * point_f);
    return mix(
        mix(dot(vec_tl, point_f - vec2(0.0, 0.0)), dot(vec_tr, point_f - vec2(1.0, 0.0)), u.x),
        mix(dot(vec_bl, point_f - vec2(0.0, 1.0)), dot(vec_br, point_f - vec2(1.0, 1.0)), u.x),
        u.y
    ) * 1.4142;
}

float cloud_noise(vec2 uv, float freq, float seed, float time) {
    vec2 wrap = vec2(24.0, 24.0);
    vec2 move = vec2(-time, -time) * wrap;
    float v1 = abs(perlin_noise(uv * freq * 1.0 + move * vec2(-1.0, 1.0), wrap, seed));
    float v2 = abs(perlin_noise(uv * freq * 2.0 + move * 1.5, wrap * 1.5, seed + wrap.x)) * 0.5;
    float v3 = abs(perlin_noise(uv * freq * 4.0 + move * 2.0, wrap * 2.0, seed + wrap.x * 2.5)) * 0.25;

    float x_mul = min(smoothstep(0.0, 0.5, uv.x), smoothstep(1.0, 0.5, uv.x));
    float y_mul = min(smoothstep(0.0, 0.2, uv.y), smoothstep(1.0, 0.2, uv.y));

    return (v1 + v2 + v3) * x_mul * y_mul * 1.0 - 0.1;
}

void main() {

    vec3 light_dir = vec3(cos(u_time * 0.1), sin(u_time * 0.1), 0.0);
    if (light_pos.z > 0.0) {
        vec2 center = v_texindex.xy / 2.0;
        light_dir = vec3(normalize(light_pos.xy - center), 0.0);
    }

    float morph_time = 200.0;
    float freq = 1.5;

    vec2 uv = vec2(v_texindex.x, 1.0 - v_texindex.y);;

    float height = cloud_noise(uv, freq, 0.0, mod(u_time / morph_time, 1.0));
    float other = cloud_noise(uv - light_dir.xy/v_texindex.xy, freq, 0.0, mod(u_time / morph_time, 1.0));
    float brightness = mix(0.8, 1.0, ceil(other - height));

    height = min(ceil(max(height, 0.0)), 1.0);

    // 云层混合计算
    float cloudFactor = smoothstep(0.3, 0.7, height); // 云层密度控制
    float shadowFactor = smoothstep(0.4, 0.6, other); // 阴影密度控制

    vec3 screen = u_groundColor;

    // vec3 cloudColor = mix(u_groundColor * 0.7, u_groundColor, shadowFactor); // 云层阴影混合
    // vec3 finalColor = mix(u_groundColor, cloudColor, cloudFactor); // 云层覆盖颜色

    FragColor = vec4(mix(screen, mix(screen, vec3(0.3), brightness), height), cloudFactor * 0.3);

    // FragColor = vec4(finalColor, cloudFactor * 0.3);

}


#end FRAGMENT

