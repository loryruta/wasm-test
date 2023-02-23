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

    const int k_nearest_point_history = 5;

    ivec2 nearest_point_idx[k_nearest_point_history];
    float nearest_point_distance[k_nearest_point_history];

    for (int i = 0; i < k_nearest_point_history; i++)
        nearest_point_distance[i] = INF;

    ivec2 texture_size = textureSize(u_point_pos, 0);

    for (int x = 0; x < texture_size.x; x++)
    {
        for (int y = 0; y < texture_size.y; y++)
        {
            vec2 point = texelFetch(u_point_pos, ivec2(x, y), 0).rg;

            float d = distance(point, my_point);
            if (d < nearest_point_distance[0])
            {
                for (int i = k_nearest_point_history - 1; i > 0; i--)
                {
                    nearest_point_distance[i] = nearest_point_distance[i - 1];
                    nearest_point_idx[i] = nearest_point_idx[i - 1];
                }

                nearest_point_distance[0] = d;
                nearest_point_idx[0] = ivec2(x, y);
            }
        }
    }

    // Point color
    int pi = nearest_point_idx[0].x * texture_size.y + nearest_point_idx[0].y;
    vec3 point_color = vec3(
        rand(float(pi + 5678)),
        rand(float(pi + 2342)),
        rand(float(pi + 8905))
    );

    const float k_border_size = 123.0;
    const float k_border_shade_stripes = 3.8;
    const vec3 k_border_color = vec3(0, 0, 0.2); 

    vec2 p1 = texelFetch(u_point_pos, nearest_point_idx[0], 0).rg;
    vec2 p2 = texelFetch(u_point_pos, nearest_point_idx[1], 0).rg;
    
    float p1d = nearest_point_distance[0];
    float p2d = nearest_point_distance[1];
    
    vec2 md = (p1 + p2) / 2.0;

    // Rounded line
    vec2 rlo = p1;
    vec2 rld = vec2(0.2, 0.2);
    //rld = vec2(rld.x > rld.y ? 1.0 : 0.0, rld.y > rld.x ? 1.0 : 0.0);

    float rla = rld.y;
    float rlb = -rld.x;
    float rlc = -rld.y * rlo.x + rlo.y * rld.x;

    float ld = abs(rla * my_point.x + rlb * my_point.y + rlc) / sqrt(rla * rla + rlb * rlb);

    // Light incidence
    const vec3 k_light_dir = normalize(vec3(-1.0, -1.0, -1.0));

    // Border distance
    float bd = abs(p1d - p2d);
    bd = min(bd, 100.0) / 100.0;
    //bd = bd - mod(bd, 0.25);
    float ni = mix(0.2, 0.9, bd); // Normal inclination
    vec3 n = normalize(vec3(normalize(p2 - p1), ni));

    float dot_ = dot(-k_light_dir, n);

    float a = 0.0;

    float le = step(0.44, dot_); // Light enable
    float li = (dot_ - 0.14) / (1.0 - 0.14) * 0.5 + 0.5;
    li = li - mod(li, 0.16);
    a += le * li * 0.99;

    float se = 1.0 - le; // Shadow enable
    float si = min(1.0 + dot_, 1.0); // Shadow influence
    si = si - mod(si, 0.16);
    a += se * si * 0.44;

    vec3 shade = a * point_color;
    frag_color = vec4(shade, 1.0);
    //frag_color = vec4(vec3(ni), 1.0);
    //frag_color = vec4(vec3(rand(bd)), 1.0);

    //if (bd < 8.0)
    //    frag_color = vec4(0, 0, 0.11, 1);

    // SHOW POINTS
    //if (p1d < 5.0)
    //    frag_color = vec4(1, 1, 1, 1);
}
