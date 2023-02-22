#version 300 es

precision highp float;

uniform sampler2D u_point_pos;
uniform sampler2D u_point_dir;

uniform float u_dt;
uniform vec2 u_screen_size;

layout(location = 0) out vec2 out_point_pos;
layout(location = 1) out vec2 out_point_dir;

void update_point(
    inout vec2 point_pos,
    inout vec2 point_dir
)
{
    const float k_speed = 2.0;
    const float k_margin = 1.0;

    point_pos += point_dir * k_speed;

    if (point_pos.x < 0.0)
    {
        point_dir.x = -point_dir.x;
        point_pos.x = k_margin;
    }

    if (point_pos.x > u_screen_size.x)
    {
        point_dir.x = -point_dir.x;
        point_pos.x = u_screen_size.x - k_margin;
    }

    if (point_pos.y < 0.0)
    {
        point_dir.y = -point_dir.y;
        point_pos.y = k_margin;
    }

    if (point_pos.y > u_screen_size.y)
    {
        point_dir.y = -point_dir.y;
        point_pos.y = u_screen_size.y - k_margin;
    }

    //if (abs(point_dir.x) < 0.1) point_dir.x = sign(point_dir.x) * 0.1;
    //if (abs(point_dir.y) < 0.1) point_dir.x = sign(point_dir.y) * 0.1;

    //point_pos = vec2(u_screen_size) / vec2(2);
}

void main()
{
    vec2 point_pos = texelFetch(u_point_pos, ivec2(gl_FragCoord.xy), 0).rg;
    vec2 point_dir = texelFetch(u_point_dir, ivec2(gl_FragCoord.xy), 0).rg;

    update_point(point_pos, point_dir);
 
    out_point_pos = point_pos;
    out_point_dir = point_dir;
}
