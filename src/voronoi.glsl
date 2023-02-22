#version 300 es

#if (__NUM_POINTS__ % 2 != 0)
#   error "__NUM_POINTS__" must be a multiple of 2!
#endif

#define INF        1e35;
#define UINT32_MAX 0xFFFFFFFFu

precision highp float;
precision highp int;

uniform KDTreeBlock
{
    uvec4 kdtree_nodes[__NUM_KDTREE_NODES__];
};

uniform KDTreePointsBlock
{
    vec4 kdtree_points[__NUM_POINTS__ / 2];
};

uvec4 get_kdtree_node(uint i)
{
    return kdtree_nodes[i];
}

vec2 get_kdtree_point(uint i)
{
    vec4 v = kdtree_points[i / 2u];
    return i % 2u == 0u ? v.xy : v.zw;
}

bool kdtree_is_leaf(uvec4 node)
{
    return node.y == UINT32_MAX;
}

struct kdtree_search_stack_entry
{
    uint node_idx;
    bool has_visited_left;
    bool has_visited_right;
};

uint kdtree_search(
    vec2 my_point,
    out uint nearest_point_idx,
    out float nearest_distance_squared
)
{
    nearest_point_idx = UINT32_MAX;
    nearest_distance_squared = INF;

    uint num_iterations = 0u;

    int stack_idx = 0;
    kdtree_search_stack_entry stack[32];

    kdtree_search_stack_entry current;

    current.node_idx = 0u;
    current.has_visited_left = false;
    current.has_visited_right = false;

    while (true)
    {
        num_iterations++;

        uvec4 node = kdtree_nodes[current.node_idx];

        if (kdtree_is_leaf(node))
        {
            uint point_idx = node.x;
            uint num_points = node.z;

            // Search for the nearest point within this split
            for (uint i = point_idx; i < point_idx + num_points; i++)
            {
                vec2 p = get_kdtree_point(i);
                float distance_squared = dot(my_point - p, my_point - p);

                if (nearest_distance_squared > distance_squared)
                {
                    nearest_point_idx = i;
                    nearest_distance_squared = distance_squared;
                }
            }
        }
        else // Parent node
        {
            float node_split = uintBitsToFloat(node.x);
            uint node_axis = node.y;

            float signed_distance = my_point[node_axis] - node_split;
            float signed_distance_squared = signed_distance * signed_distance;

            // Enter left node
            if (!current.has_visited_left)
            {
                current.has_visited_left = true;

                // PUSH
                stack[stack_idx] = current;
                stack_idx++;

                current.node_idx = current.node_idx + 1u;
                current.has_visited_left = false;
                current.has_visited_right = false;

                continue;
            }

            // Try to enter right node
            if (!current.has_visited_right)
            {
                current.has_visited_right = true;
                
                 // Actually visit right node only if the split is nearest than the nearest distance found
                if (abs(signed_distance_squared) < nearest_distance_squared)
                {
                    // PUSH
                    stack[stack_idx] = current;
                    stack_idx++;

                    current.node_idx = current.node_idx + /* right_node_distance */ node.z;
                    current.has_visited_left = false;
                    current.has_visited_right = false;

                    stack_idx++;

                    continue;
                }
            }

            // POP
            if (stack_idx == 0)
                break;

            current = stack[stack_idx];
            stack_idx--;
        }
    }

    return num_iterations;
}

out vec4 f_color;

void main()
{
    uint nearest_point_idx;
    float nearest_distance_squared;
    uint num_iterations;

    vec2 my_point = gl_FragCoord.xy;
    num_iterations = kdtree_search(
        my_point,
        nearest_point_idx,
        nearest_distance_squared
    );

    f_color = vec4(float(num_iterations) / 10.0, 0, 0, 1);
}
