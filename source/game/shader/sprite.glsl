
#begin VERTEX

#version 150

in vec3 wmat1; // columns 1, 2, 3 of transform matrix
in vec3 wmat2;
in vec3 wmat3;
in vec2 size;
in vec2 texcell;
in vec2 texsize;

out mat3 wmat;
out vec2 size_;
out vec2 texcell_;
out vec2 texsize_;

void main()
{
    wmat = mat3(wmat1, wmat2, wmat3);
    size_ = size;
    texcell_ = texcell;
    texsize_ = texsize;
}



#end VERTEX

#begin FRAGMENT

#version 150

in vec2 texcoord;

uniform sampler2D tex0;

out vec4 outColor;

void main()
{
    outColor = texture(tex0, texcoord);
}



#end FRAGMENT

#begin GEOMETRY

#version 150

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in mat3 wmat[];
in vec2 size_[];
in vec2 texcell_[];
in vec2 texsize_[];

out vec2 texcoord;

uniform mat3 inverse_view_matrix;

uniform vec2 atlas_size;

void main()
{
    mat3 m = inverse_view_matrix * wmat[0];
    vec3 sm = vec3(size_[0], 1.0);

    gl_Position = vec4(m * (sm * vec3(-0.5, 0.5, 1.0)), 1.0);
    texcoord = (texcell_[0] + texsize_[0] * vec2(0.0, 1.0)) / atlas_size;
    EmitVertex();

    gl_Position = vec4(m * (sm * vec3(-0.5, -0.5, 1.0)), 1.0);
    texcoord = (texcell_[0] + texsize_[0] * vec2(0.0, 0.0)) / atlas_size;
    EmitVertex();

    gl_Position = vec4(m * (sm * vec3(0.5, 0.5, 1.0)), 1.0);
    texcoord = (texcell_[0] + texsize_[0] * vec2(1.0, 1.0)) / atlas_size;
    EmitVertex();

    gl_Position = vec4(m * (sm * vec3(0.5, -0.5, 1.0)), 1.0);
    texcoord = (texcell_[0] + texsize_[0] * vec2(1.0, 0.0)) / atlas_size;
    EmitVertex();

    EndPrimitive();
}




#end GEOMETRY