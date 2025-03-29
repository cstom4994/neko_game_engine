#nekoshader version 1
#begin VERTEX
@version 330

@attribute vec2 position;
@attribute vec2 uv;

@varying vec2 out_uv;

@uniform mat4 projection;
@uniform mat4 transform;

void main() {
	gl_Position = projection * transform * vec4(position, 0.0, 1.0);
	out_uv = uv;
}

#end VERTEX

#begin FRAGMENT
@version 330

@varying vec2 out_uv;
@out vec4 color;

@uniform sampler2D image;

void main() {
	color = texture(image, out_uv);
}

#end FRAGMENT