#version 330 core

out vec4 color;

in VS_OUT {
	vec2 uv;
	vec4 color;
	float mode;
} fs_in;

uniform sampler2D atlas;
uniform sampler2D font;

void main() {
	vec4 texture_color = vec4(1.0);

	int mode = int(fs_in.mode);

	if (mode == 1) {
		texture_color = vec4(texture(atlas, fs_in.uv).r);
	} else if (mode == 2) {
		texture_color = vec4(vec3(1.0), texture(font, fs_in.uv).r); 
	}

	color = texture_color * fs_in.color;
}