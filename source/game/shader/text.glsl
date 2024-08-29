
#begin VERTEX

#version 150

in vec2 pos;
in vec2 cell;
in float is_cursor;

out vec2 pos_;
out vec2 cell_;
out float is_cursor_;

void main()
{
    pos_ = pos;
    cell_ = cell;
    is_cursor_ = is_cursor;
}

#end VERTEX

#begin FRAGMENT

#version 150

in vec2 texcoord;
in float is_cursor;

uniform sampler2D tex0;
uniform vec4 base_color;
uniform float cursor_blink;

out vec4 outColor;

void main()
{
    if (is_cursor > 0)
        outColor = vec4(base_color.xyz, cursor_blink);
    else
        outColor = base_color * texture(tex0, texcoord);
}



#end FRAGMENT

#begin GEOMETRY

#version 150

layout(points) in; // 输入的图元类型是点
layout(triangle_strip, max_vertices = 4) out; // 输出的图元类型是三角形带 triangle_strip 并且最多输出4个顶点

in vec2 pos_[];
in vec2 cell_[];
in float is_cursor_[];

out vec2 texcoord;
out float is_cursor;

uniform mat3 inverse_view_matrix;
uniform mat3 wmat;

uniform vec2 size;
uniform vec2 inv_grid_size;

void main()
{
    is_cursor = is_cursor_[0];

    mat3 m = inverse_view_matrix * wmat;
    vec2 offset = size * pos_[0];

    gl_Position = vec4(m * vec3(offset + vec2(   0.0, size.y), 1.0), 1.0);
    texcoord = inv_grid_size * (cell_[0] + vec2(0.0, 1.0));
    EmitVertex();

    gl_Position = vec4(m * vec3(offset + vec2(   0.0,    0.0), 1.0), 1.0);
    texcoord = inv_grid_size * (cell_[0] + vec2(0.0, 0.0));
    EmitVertex();

    gl_Position = vec4(m * vec3(offset + vec2(size.x, size.y), 1.0), 1.0);
    texcoord = inv_grid_size * (cell_[0] + vec2(1.0, 1.0));
    is_cursor = -3; /* mad hax for vertical line cursor */
    EmitVertex();

    gl_Position = vec4(m * vec3(offset + vec2(size.x,    0.0), 1.0), 1.0);
    texcoord = inv_grid_size * (cell_[0] + vec2(1.0, 0.0));
    is_cursor = -3;
    EmitVertex();

    EndPrimitive();
}



#end GEOMETRY