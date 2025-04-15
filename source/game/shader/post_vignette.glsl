#begin VERTEX
#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

out vec2 vert_pos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    TexCoords = aTexCoords;
	vert_pos = aPos;
}

#end VERTEX

#begin FRAGMENT

#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int enable;

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

float pixel_w = 3.0; // 15.0
float pixel_h = 2.0; // 10.0

uniform vec2 NekoScreenSize;
uniform sampler2D NekoTextureInput;


void main()
{

     float rt_w = NekoScreenSize.x;
     float rt_h = NekoScreenSize.y;

    vec4 pixel;

    if(enable==1){
        vec2 uv = TexCoords;

        // vec4 pixel = texelFetch(NekoTextureInput, ivec2(vert_pos.xy), 0);
        pixel = texture(NekoTextureInput, uv);
        float effect = pow(distance(vert_pos / start, vec2(0)), intensity);
        pixel = vec4(mix(pixel.xyz, vec3(0), effect), pixel.w);

        // 伽马矫正
        pixel = vec4(pow(pixel.xyz, vec3(1) / power), pixel.w);
    }else if(enable==2){
        vec2 uv = TexCoords;

        float vx_offset = 0.5;

        vec3 tc = vec3(1.0, 0.0, 0.0);
        if (uv.x < (vx_offset-0.005))
        {
            float dx = pixel_w*(1./rt_w);
            float dy = pixel_h*(1./rt_h);
            vec2 coord = vec2(dx*floor(uv.x/dx),dy*floor(uv.y/dy));
            tc = texture(NekoTextureInput, coord).rgb;
        }
        else if (uv.x>=(vx_offset+0.005))
        {
            tc = texture(NekoTextureInput, uv).rgb;
        }

        pixel = vec4(tc, 1.0);
    }else {
        pixel = texture(NekoTextureInput, TexCoords);
    }

    // // 高斯模糊
    // vec3 blur_pixel = vec3(0.0);
    // ivec2 texSize = textureSize(NekoTextureInput, 0); // 获取纹理的尺寸
    // ivec2 center = ivec2(TexCoords * vec2(texSize)); // 将 TexCoords 转换为纹理像素位置
    // // 遍历高斯核的 5x5 区域
    // for (int i = -2; i <= 2; ++i) {
    //     for (int j = -2; j <= 2; ++j) {
    //         int kernelIndex = (i + 2) * 5 + (j + 2); // 计算内核的索引
    //         ivec2 offset = ivec2(i, j); // 偏移
    //         blur_pixel += texelFetch(NekoTextureInput, center + offset, 0).rgb * kernel[kernelIndex];
    //     }
    // }
    // FragColor = vec4(blur_pixel, pixel.a);



	FragColor = pixel;
}

#end FRAGMENT
