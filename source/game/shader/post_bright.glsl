#begin VERTEX

#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;
out vec2 tex_coord;
void main() {
   gl_Position = vec4(a_position, 0.0, 1.0);
   tex_coord = a_uv;
}

#end VERTEX

#begin FRAGMENT

#version 330 core

in vec2 tex_coord;

out vec4 frag_color;

uniform vec2 NekoScreenSize;
uniform sampler2D NekoTextureInput;

void main() {
   frag_color = vec4(0.0, 0.0, 0.0, 1.0);
   vec3 tex_color = texture(NekoTextureInput, tex_coord).rgb;
   float brightness = dot(tex_color, vec3(0.2126, 0.7152, 0.0722));
   float saturation = max(tex_color.r, max(tex_color.g, tex_color.b)) - min(tex_color.r, min(tex_color.g, tex_color.b));
   if (tex_color.r > 0.6 && tex_color.g > 0.6 && tex_color.b < 0.3 && brightness > 0.5 && saturation > 0.2) {
      vec3 op = clamp(tex_color, vec3(0.0), vec3(1.0));
      frag_color = vec4(op * 0.8, 1.0);
   }
}


#end FRAGMENT
