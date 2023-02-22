#version 300 es

precision highp float;

uniform sampler2D u_point_pos;
uniform sampler2D u_point_dir;

uniform float u_dt;
uniform vec2 u_screen_size;

layout(location = 0) out vec2 out_point_pos;
layout(location = 1) out vec2 out_point_dir;

void update_point(inout vec2 point_pos, inout vec2 point_dir)
{
    if (point_pos.x < 0.0)
        { point_dir.x = -point_dir.x; point_pos.x = 0.0; }

    if (point_pos.x > u_screen_size.x)
        { point_dir.x = -point_dir.x; point_pos.x = u_screen_size.x; }

    if (point_pos.y < 0.0)
        { point_dir.y = -point_dir.y; point_pos.y = 0.0; }

    if (point_pos.y > u_screen_size.y)
        { point_dir.y = -point_dir.y; point_pos.y = u_screen_size.y; }

    point_pos += point_dir * u_dt;
}

void main()
{
    ivec2 texture_size = textureSize(u_point_pos, 0);
    vec2 uv = vec2(gl_FragCoord.xy) / vec2(texture_size);

    vec2 point_pos = texture(u_point_pos, uv).rg;
    vec2 point_dir = texture(u_point_dir, uv).rg;

    update_point(point_pos, point_dir);

    out_point_pos = point_pos;
    out_point_dir = point_dir;
}
