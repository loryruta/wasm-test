#version 300 es

precision highp float;

void main()
{
    vec2 screen_quad[6] = vec2[](
        vec2(-1.0, -1.0),
        vec2(-1.0, 1.0),
        vec2(1.0, 1.0),
        vec2(1.0, 1.0),
        vec2(1.0, -1.0),
        vec2(-1.0, -1.0)
    );
    gl_Position = vec4(screen_quad[gl_VertexID], 0, 1);
}
