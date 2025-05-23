
#begin VERTEX

#version 150

in vec3 wmat1; // columns 1, 2, 3 of transform matrix
in vec3 wmat2;
in vec3 wmat3;

in vec2 bbmin;
in vec2 bbmax;
in float selected;

out mat3 wmat;
out vec2 bbmin_;
out vec2 bbmax_;
out float selected_;

void main()
{
    wmat = mat3(wmat1, wmat2, wmat3);
    bbmin_ = bbmin;
    bbmax_ = bbmax;
    selected_ = selected;
}


#end VERTEX

#begin FRAGMENT

#version 150

in float selected;

out vec4 color;

uniform float is_grid;

void main()
{
    if (is_grid > 0.5)
        color = vec4(0.5, 0.5, 0.5, 0.17);
    else if (selected > 0.5)
        color = vec4(1, 0, 0, 1);
    else
        color = vec4(0, 0.7, 0, 1);
}


#end FRAGMENT

#begin GEOMETRY

#version 150

layout(points) in;
layout(line_strip, max_vertices = 14) out;

in mat3 wmat[];
in vec2 bbmin_[];
in vec2 bbmax_[];
in float selected_[];

uniform mat3 inverse_view_matrix;
uniform float aspect;

out float selected;

void main()
{
    selected = selected_[0];
    mat3 m = inverse_view_matrix * wmat[0];

    /* draw bbox */
    vec2 bbmin = bbmin_[0];
    vec2 bbmax = bbmax_[0];
    gl_Position = vec4(m * vec3(bbmin.x, bbmax.y, 1.0), 1.0);
    EmitVertex();
    gl_Position = vec4(m * vec3(bbmin.x, bbmin.y, 1.0), 1.0);
    EmitVertex();
    gl_Position = vec4(m * vec3(bbmax.x, bbmin.y, 1.0), 1.0);
    EmitVertex();
    gl_Position = vec4(m * vec3(bbmax.x, bbmax.y, 1.0), 1.0);
    EmitVertex();
    gl_Position = vec4(m * vec3(bbmin.x, bbmax.y, 1.0), 1.0);
    EmitVertex();
    EndPrimitive();

    /* draw more stuff if selected */
    if (selected > 0.5)
    {
        /* screen-space coordinates */
        vec2 o = (m * vec3(0, 0, 1)).xy;       /* origin */
        vec2 u = (m * vec3(0, 1, 1)).xy - o;   /* up */
        vec2 r = (m * vec3(1, 0, 1)).xy - o;   /* right */

        /* normalize using aspect-corrected metric */
        u /= sqrt(aspect * aspect * u.x * u.x + u.y * u.y);
        r /= sqrt(aspect * aspect * r.x * r.x + r.y * r.y);

        /* draw axes */
        gl_Position = vec4(o, 0, 1);
        EmitVertex();
        gl_Position = vec4(o + 0.06 * u, 0, 1);
        EmitVertex();
        EndPrimitive();
        gl_Position = vec4(o, 0, 1);
        EmitVertex();
        gl_Position = vec4(o + 0.06 * r, 0, 1);
        EmitVertex();
        EndPrimitive();

        /* draw grown bbox */
        vec2 g1 = 0.02 * (u + r);
        vec2 g2 = 0.02 * (u - r);
        gl_Position = vec4(m * vec3(bbmin.x, bbmax.y, 1) + vec3( g2, 0), 1);
        EmitVertex();
        gl_Position = vec4(m * vec3(bbmin.x, bbmin.y, 1) + vec3(-g1, 0), 1);
        EmitVertex();
        gl_Position = vec4(m * vec3(bbmax.x, bbmin.y, 1) + vec3(-g2, 0), 1);
        EmitVertex();
        gl_Position = vec4(m * vec3(bbmax.x, bbmax.y, 1) + vec3( g1, 0), 1);
        EmitVertex();
        gl_Position = vec4(m * vec3(bbmin.x, bbmax.y, 1) + vec3( g2, 0), 1);
        EmitVertex();
        EndPrimitive();
    }
}



#end GEOMETRY