#version 300 es

precision highp float;

#define INF        1e35;
#define UINT32_MAX 0xFFFFFFFFu

uniform sampler2D u_point_pos;

out vec4 frag_color;

float hash(float n)
{
    return fract(sin(n) * 43758.5453123);
}

void main()
{
    vec2 my_point = gl_FragCoord.xy;

    ivec2 nearest_point_idx[2];
    float nearest_point_distance[2];

    nearest_point_distance[0] = INF;
    nearest_point_distance[1] = INF;

    ivec2 texture_size = textureSize(u_point_pos, 0);

    for (int x = 0; x < texture_size.x; x++)
    {
        for (int y = 0; y < texture_size.y; y++)
        {
            vec2 uv = vec2(x, y) / vec2(texture_size);
            vec2 point = texture(u_point_pos, uv).rg;

            float d = distance(point, my_point);
            for (int k = 0; k < 2; k++)
            {
                if (d < nearest_point_distance[k])
                {
                    nearest_point_distance[k] = d;
                    nearest_point_idx[k] = ivec2(x, y);

                    break;
                }
            }
        }
    }

    frag_color = vec4(vec3(abs(sin(nearest_point_distance[0] * 0.1))), 1);

}
