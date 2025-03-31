#nekoshader version 1
#begin VERTEX

@version 330

@attribute vec2 position;
@attribute float point_size;
@attribute vec4 color;

@varying vec4 color_;

@uniform mat3 inverse_view_matrix;

void main()
{
    gl_Position = vec4(inverse_view_matrix * vec3(position, 1.0), 1.0);
    gl_PointSize = point_size;
    color_ = color;
}

#end VERTEX

#begin FRAGMENT

@version 330

@varying vec4 color_;

@out vec4 outColor;

void main()
{
    outColor = color_;
}

#end FRAGMENT
