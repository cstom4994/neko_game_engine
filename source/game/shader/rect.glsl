
#begin VERTEX

#version 150

in vec3 wmat1; // columns 1, 2, 3 of transform matrix
in vec3 wmat2;
in vec3 wmat3;

in vec2 size;
in vec4 color;
in int visible;

out mat3 wmat;
out vec2 size_;
out vec4 color_;
out int visible_;

void main()
{
    wmat = mat3(wmat1, wmat2, wmat3);
    size_ = size;
    color_ = color;
    visible_ = visible;
}

#end VERTEX

#begin FRAGMENT

#version 150

in vec4 color;

out vec4 outColor;

void main()
{
    outColor = color;
}


#end FRAGMENT

#begin GEOMETRY

#version 150

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in mat3 wmat[];
in vec2 size_[];
in vec4 color_[];
in int visible_[];

out vec4 color;

uniform mat3 inverse_view_matrix;

void main()
{
    if (visible_[0] == 0)
        return;

    color = color_[0];
    mat3 m = inverse_view_matrix * wmat[0];

    gl_Position = vec4(m * vec3(0, -size_[0].y, 1.0), 1.0);
    EmitVertex();
    gl_Position = vec4(m * vec3(0, 0, 1.0), 1.0);
    EmitVertex();
    gl_Position = vec4(m * vec3(size_[0].x, -size_[0].y, 1.0), 1.0);
    EmitVertex();
    gl_Position = vec4(m * vec3(size_[0].x, 0, 1.0), 1.0);
    EmitVertex();
    EndPrimitive();
}



#end GEOMETRY