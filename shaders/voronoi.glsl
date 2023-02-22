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

    uint nearest_point_idx[2];
    float nearest_point_distance[2];

    nearest_point_distance[0] = INF;
    nearest_point_distance[1] = INF;

    ivec2 texture_size = textureSize(u_point_pos, 0);

    for (int x = 0; x < texture_size.x; x++)
    {
        for (int y = 0; y < texture_size.y; y++)
        {
            vec2 point = texelFetch(u_point_pos, ivec2(x, y), 0).rg;

            float d = distance(point, my_point);
            if (d < nearest_point_distance[0])
            {
                nearest_point_distance[1] = nearest_point_distance[0];
                nearest_point_idx[1] = nearest_point_idx[0];

                nearest_point_distance[0] = d;
                nearest_point_idx[0] = uint(x * texture_size.y + y);
            }
        }
    }

    vec3 point_color = vec3(
        hash(float(nearest_point_idx[0] + 324u)),
        hash(float(nearest_point_idx[0] + 893u)),
        hash(float(nearest_point_idx[0] + 172u))
    );

    const float k_border_size = 80.0;
    const float k_border_shade_stripes = 3.8;
    const vec3 k_border_color = vec3(0, 0, 0.07); 

    float ed = abs(nearest_point_distance[0] - nearest_point_distance[1]);
    ed = min(ed, k_border_size) / k_border_size;
    ed = ed - mod(ed, (1.0 / k_border_shade_stripes));

    // SHOW VORONOI
    frag_color = vec4(
        mix(k_border_color, point_color, ed),
        1
    );

    /*
    // SHOW POINTS
    if (nearest_point_distance[0] < 15.0)
        frag_color = vec4(0, 0, 0, 1);
    else
        frag_color = vec4(point_color, 1);*/
}
