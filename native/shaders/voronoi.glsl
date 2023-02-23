#version 300 es

precision highp float;

#define INF        1e35;
#define UINT32_MAX 0xFFFFFFFFu

uniform sampler2D u_point_pos;

out vec4 frag_color;

float rand(float n){return fract(sin(n) * 43758.5453123);}

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
            vec2 point = texelFetch(u_point_pos, ivec2(x, y), 0).rg;

            float d = distance(point, my_point);
            if (d < nearest_point_distance[0])
            {
                nearest_point_distance[1] = nearest_point_distance[0];
                nearest_point_idx[1] = nearest_point_idx[0];

                nearest_point_distance[0] = d;
                nearest_point_idx[0] = ivec2(x, y);
            }
        }
    }

    int pi = nearest_point_idx[0].x * texture_size.y + nearest_point_idx[0].y;

    vec3 point_color = vec3(
        rand(float(pi + 5678)) * 0.6 + 0.2,
        rand(float(pi + 2342)) * 0.6 + 0.2,
        rand(float(pi + 8905)) * 0.6 + 0.2
    );

    const float k_border_size = 123.0;
    const float k_border_shade_stripes = 3.8;
    const vec3 k_border_color = vec3(0, 0, 0.2); 

    vec2 p1 = texelFetch(u_point_pos, nearest_point_idx[0], 0).rg;
    vec2 p2 = texelFetch(u_point_pos, nearest_point_idx[1], 1).rg;
    vec2 md = (p1 + p2) / 2.0;

    float d1 = nearest_point_distance[0];
    float d2 = nearest_point_distance[1];

    float ed = abs(d1 - d2);
    //float cd = abs((p2.x - p1.x) * (p1.y - my_point.y) - (p1.x - my_point.x) * (p2.y - p1.y)) / distance(p2, p1);

    float fd = ed;
    fd = min(fd, k_border_size) / k_border_size;
    fd = fd - mod(fd, (1.0 / k_border_shade_stripes));

    // SHOW VORONOI
    frag_color = vec4(
        mix(k_border_color, point_color, fd),
        1
    );

    /*
    // SHOW POINTS
    if (nearest_point_distance[0] < 15.0)
        frag_color = vec4(0, 0, 0, 1);
    else
        frag_color = vec4(point_color, 1);*/
}
