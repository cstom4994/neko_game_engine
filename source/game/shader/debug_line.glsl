
#begin VERTEX

#version 450

layout(location = 0) in vec4 pos_width;
layout(location = 1) in vec4 col;

layout(location = 0) uniform mat3 inverse_view_matrix;
layout(location = 1) uniform vec2 u_viewport_size;

out vec4 v_col;
out noperspective float v_line_width;
out vec3 v_world_pos;

void main()
{
    v_col = col;
    v_line_width = pos_width.w;

    vec3 pos = pos_width.xyz;
    // pos.y *= -1.0; // 反转y轴
    
    vec3 world_pos = inverse_view_matrix * pos;
    world_pos.xy += inverse_view_matrix[2].xy; // 视口偏移
    
    v_world_pos = world_pos;
    gl_Position = vec4(world_pos, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 450

layout(location = 2) uniform vec2 u_aa_radius;

in vec4 g_col;
in noperspective float g_u;
in noperspective float g_v;
in noperspective float g_line_width;
in noperspective float g_line_length;

out vec4 frag_color;
void main()
{
    // 我们渲染一个加宽了 r 的四边形 使得线条的总宽度为 w+r
    // 我们希望在宽度 w 周围进行平滑处理 以便边缘能够被适当地平滑化
    // 因此在 smoothstep 函数中 我们有以下定义
    // 远边缘 FarEdge : 1.0 = (w+r) / (w+r)
    // 近边缘 CloseEdge : 1.0 - (2r / (w+r)) = (w+r)/(w+r) - 2r/(w+r)) = (w-r) / (w+r)
    // 这样平滑处理就集中在宽度 w 周围
    
    float au = 1.0 - smoothstep( 1.0 - ((2.0*u_aa_radius[0]) / g_line_width),  1.0, abs(g_u / g_line_width) );
    float av = 1.0 - smoothstep( 1.0 - ((2.0*u_aa_radius[1]) / g_line_length), 1.0, abs(g_v / g_line_length) );
    frag_color = g_col;
    frag_color.a *= min(av, au);
}

#end FRAGMENT


#begin GEOMETRY

#version 450

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 1) uniform vec2 u_viewport_size;     // 视口尺寸
layout(location = 2) uniform vec2 u_aa_radius;        // 抗锯齿半径

in vec4 v_col[];                    // 顶点颜色
in noperspective float v_line_width[]; // 顶点线条宽度
in vec3 v_world_pos[];              // 顶点世界空间位置

out vec4 g_col;
out noperspective float g_line_width;
out noperspective float g_line_length;
out noperspective float g_u;
out noperspective float g_v;

void main()
{
    // 直接使用传入的世界坐标
    vec2 ndc_a = v_world_pos[0].xy;
    vec2 ndc_b = v_world_pos[1].xy;

    // 计算线段方向及其在视口中的长度
    float aspect_ratio = u_viewport_size.x / u_viewport_size.y;
    vec2 line_vector = ndc_b - ndc_a;
    vec2 viewport_line_vector = line_vector * u_viewport_size;
    vec2 dir = normalize(vec2(line_vector.x, line_vector.y * aspect_ratio));

    // 计算线宽与抗锯齿扩展
    float line_width_a = max(1.0, v_line_width[0]) + u_aa_radius.x;
    float line_width_b = max(1.0, v_line_width[1]) + u_aa_radius.x;
    float extension_length = u_aa_radius.y;
    float line_length = length(viewport_line_vector) + 2.0 * extension_length;
    vec2 inv_viewport = 1.0 / u_viewport_size;

    // 计算法线和扩展向量
    vec2 normal = vec2(-dir.y, dir.x);
    vec2 normal_a = normal * line_width_a * inv_viewport;
    vec2 normal_b = normal * line_width_b * inv_viewport;
    vec2 extension = dir * extension_length * inv_viewport;

    // 顶点 1
    g_col = vec4(v_col[0].rgb, v_col[0].a * min(v_line_width[0], 1.0));
    g_u = line_width_a;
    g_v = line_length * 0.5;
    g_line_width = line_width_a;
    g_line_length = line_length * 0.5;
    gl_Position = vec4(ndc_a + normal_a - extension, 0.0, 1.0);
    EmitVertex();

    // 顶点 2
    g_u = -line_width_a;
    g_v = line_length * 0.5;
    g_line_width = line_width_a;
    g_line_length = line_length * 0.5;
    gl_Position = vec4(ndc_a - normal_a - extension, 0.0, 1.0);
    EmitVertex();

    // 顶点 3
    g_col = vec4(v_col[1].rgb, v_col[1].a * min(v_line_width[1], 1.0));
    g_u = line_width_b;
    g_v = -line_length * 0.5;
    g_line_width = line_width_b;
    g_line_length = line_length * 0.5;
    gl_Position = vec4(ndc_b + normal_b + extension, 0.0, 1.0);
    EmitVertex();

    // 顶点 4
    g_u = -line_width_b;
    g_v = -line_length * 0.5;
    g_line_width = line_width_b;
    g_line_length = line_length * 0.5;
    gl_Position = vec4(ndc_b - normal_b + extension, 0.0, 1.0);
    EmitVertex();

    EndPrimitive();
}



#end GEOMETRY