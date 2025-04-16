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

uniform sampler2D tex0;

uniform mat3 inverse_view_matrix;
uniform vec3 u_groundColor = vec3(0.1, 0.1, 0.1);

out vec4 FragColor;

uniform vec2 NekoScreenSize;

const float MAX_BLADE_LENGTH = 10.0;

uniform vec4 uTipColor = vec4(0.6, 0.9, 0.3, 1.0);  // RGB(153, 230, 77)
uniform vec4 uWindColor = vec4(0.3, 0.5, 0.4, 1.0); // RGB(77, 128, 102)
uniform vec4 uRootColor = vec4(0.2, 0.5, 0.1, 1.0);

float sampleBladeLength(vec2 uv) {
    return texture(tex0, uv).r * 255.0;
}

vec4 computeGradientColor(float distanceRatio) {
    return mix(uRootColor, uTipColor, distanceRatio);
}

// 伪随机数生成
float rand(vec2 seed) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

// 值噪声生成
float valueNoise(vec2 uv) {
    vec2 i = floor(uv);
    vec2 f = fract(uv);
    
    // 四个角点的随机值
    float a = rand(i);
    float b = rand(i + vec2(1.0, 0.0));
    float c = rand(i + vec2(0.0, 1.0));
    float d = rand(i + vec2(1.0, 1.0));
    
    // 双线性插值
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + 
           (c - a) * u.y * (1.0 - u.x) + 
           (d - b) * u.x * u.y;
}

void main() {

    vec2 screenUV = gl_FragCoord.xy / NekoScreenSize;
    vec2 pixelSize = 8.0 / NekoScreenSize;

    vec3 c = vec3(0.1, 0.5, 0.9);

    vec2 uv = vec2(v_texindex.x, v_texindex.y);;

    float noise = valueNoise(uv * 1.0 + vec2(u_time * 4.0)); // 缩放+动画
    // uv -= vec2(0.0, pixelSize.y * noise * 0.2); // 扰动UV
    uv -= vec2(0.0f, noise * 0.1);

    // uv = (floor((uv * 128.0) / 2.0) * 2.0 + 1.0) / 128.0;

    if (texture(tex0, uv).r > 0.0) {
        // FragColor = sampleColor(0.0);
        // FragColor.rgb -= texture(uCloudTex, screenUV).rgb;
        // FragColor = vec4(c, texture(tex0, uv).r);

        // return;

    } else {
        FragColor = vec4(0.0);
        return;
    }

    for (float dist = 0.0; dist < MAX_BLADE_LENGTH; ++dist) {
        // float windValue = wind(uv * uScreenSize, uTime);
        float bladeLength = sampleBladeLength(uv) / 25.5;
        
        if (bladeLength > 0.0) {
            // 根据距离混合tip和wind颜色 越靠近尖端越偏向tipColor
            if (abs(dist - bladeLength) < 0.5) {
                float mixFactor = smoothstep(0.0, MAX_BLADE_LENGTH, dist / bladeLength);
                FragColor = mix(uWindColor, uTipColor, mixFactor * 25.5);
                // FragColor.rgb -= texture(uCloudTex, uv).rgb;
                break;
            } else if (dist < bladeLength) {
                // 草茎部分仍然使用渐变颜色
                // FragColor = computeGradientColor(dist);
                // FragColor.rgb -= texture(uCloudTex, uv).rgb;

                FragColor = vec4(uWindColor.rgb, 1.0);
            }
        }
        
        uv -= vec2(0.0, pixelSize.y);
    }

}


#end FRAGMENT

