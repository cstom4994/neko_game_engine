#begin VERTEX
#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out vec2 vert_pos;

// 电影效果
out vec2 vert_uv;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    TexCoords = aTexCoords;
	vert_pos = aPos;
	vert_uv = vert_pos / 2 + 0.5f;
}

#end VERTEX

#begin FRAGMENT

#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int enable;
uniform sampler2D screenTexture;

uniform float intensity = 2;
uniform float start = 2.0f;

in vec2 vert_pos;

// 伽马矫正
uniform float power = 1.0;

// 高斯模糊
float sum = 273;

float kernel[] = {
1.0f / sum, 4.0f / sum, 7.0f / sum, 4.0f / sum, 1.0f / sum,
4.0f / sum, 16.0f / sum, 26.0f / sum, 16.0f / sum, 4.0f / sum,
7.0f / sum, 26.0f / sum, 41.0f / sum, 26.0f / sum, 7.0f / sum,
4.0f / sum, 16.0f / sum, 26.0f / sum, 16.0f / sum, 4.0f / sum,
1.0f / sum, 4.0f / sum, 7.0f / sum, 4.0f / sum, 1.0f / sum
};

// 电影效果
in vec2 vert_uv;

uniform float chromatic_aberration_effect_start = 1;
uniform float rOffset = 0.005;
uniform float gOffset = 0.005;
uniform float bOffset = -0.005;

uniform vec2 pixel_count = vec2(512.0, 512.0 * (9.0/16.0));

void main()
{

    vec4 pixel;

    if(enable==1){
        vec2 uv = TexCoords;
        uv *= pixel_count;
        uv = floor(uv);
        uv = uv / pixel_count;

        // vec4 pixel = texelFetch(screenTexture, ivec2(vert_pos.xy), 0);
        pixel = texture(screenTexture, uv);
        float effect = pow(distance(vert_pos / start, vec2(0)), intensity);
        pixel = vec4(mix(pixel.xyz, vec3(0), effect), pixel.w);

        // 伽马矫正
        pixel = vec4(pow(pixel.xyz, vec3(1) / power), pixel.w);
    }else{
        pixel = texture(screenTexture, TexCoords);
    }

    // // 高斯模糊
    // vec3 blur_pixel = vec3(0.0);
    // ivec2 texSize = textureSize(screenTexture, 0); // 获取纹理的尺寸
    // ivec2 center = ivec2(TexCoords * vec2(texSize)); // 将 TexCoords 转换为纹理像素位置
    // // 遍历高斯核的 5x5 区域
    // for (int i = -2; i <= 2; ++i) {
    //     for (int j = -2; j <= 2; ++j) {
    //         int kernelIndex = (i + 2) * 5 + (j + 2); // 计算内核的索引
    //         ivec2 offset = ivec2(i, j); // 偏移
    //         blur_pixel += texelFetch(screenTexture, center + offset, 0).rgb * kernel[kernelIndex];
    //     }
    // }
    // FragColor = vec4(blur_pixel, pixel.a);

	// 电影效果
    // float chromatic_aberration_effect = dot(vec2(vert_pos), vec2(vert_pos)) / chromatic_aberration_effect_start;
    // pixel = vec4(
	// 	texture(screenTexture, vert_uv + rOffset * chromatic_aberration_effect).r,
	// 	texture(screenTexture, vert_uv + gOffset * chromatic_aberration_effect).g,
	// 	texture(screenTexture, vert_uv + bOffset * chromatic_aberration_effect).b,
	// 	1
    // );



	FragColor = pixel;
}

#end FRAGMENT
