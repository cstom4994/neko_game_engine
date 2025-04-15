#begin VERTEX

#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 tex_coord;
out vec2 vert_pos;

void main() {
   gl_Position = vec4(a_position, 0.0, 1.0);
   tex_coord = a_uv;

   vert_pos = a_position.xy;
}

#end VERTEX

#begin FRAGMENT

#version 330 core

in vec2 tex_coord;
out vec4 frag_color;

uniform float u_exposure;
uniform float u_gamma;
uniform float u_bloom_scalar;
uniform float u_saturation;
float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.20;

uniform float chromatic_start = 1;
uniform float chromatic_rOffset = 0.001;
uniform float chromatic_gOffset = 0.001;
uniform float chromatic_bOffset = -0.001;

in vec2 vert_pos;

uniform vec2 NekoScreenSize;
uniform sampler2D NekoTextureInput;

uniform sampler2D u_blur_tex;

vec3 uncharted_tone_map(vec3 x) {
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main() {
   // frag_color = texture(NekoTextureInput, tex_coord);
   // return;

   float effect = dot(vec2(vert_pos), vec2(vert_pos)) / chromatic_start;
   vec3 vvv = vec3(
      texture(NekoTextureInput, tex_coord + chromatic_rOffset * effect).r,
      texture(NekoTextureInput, tex_coord + chromatic_gOffset * effect).g,
      texture(NekoTextureInput, tex_coord + chromatic_bOffset * effect).b
   );

   // vec3 hdr = max(vec3(0.0), texture(NekoTextureInput, tex_coord).rgb);
   vec3 hdr = max(vec3(0.0), vvv);
   vec3 bloom = texture(u_blur_tex, tex_coord).rgb;
   hdr += bloom * u_bloom_scalar;
   vec3 result = vec3(1.0) - exp(-hdr * u_exposure);
   result = pow(result, vec3(1.0 / u_gamma));
   float lum = result.r * 0.2 + result.g * 0.7 + result.b * 0.1;
   vec3 diff = result.rgb - vec3(lum);
   frag_color = vec4(vec3(diff) * u_saturation + lum, 1.0);
   frag_color = vec4(hdr, 1.0);
}


#end FRAGMENT
