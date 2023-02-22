#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <numeric>
#include <string>
#include <regex>
#include <optional>

#include <GLES3/gl3.h>

#include "kdtree.hpp"

// MUST BE a multiple of 2
#define NUM_POINTS 1000
#define NUM_KDTREE_NODES 1024

std::string g_vertex_shader_src = R"(#version 300 es

void main()
{
    vec2 vertices[6] = vec2[6](
        vec2(-1.0, -1.0),
        vec2(-1.0, 1.0),
        vec2(1.0, 1.0),
        vec2(1.0, 1.0),
        vec2(1.0, -1.0),
        vec2(-1.0, -1.0)
    );

    gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}
)";

std::string g_fragment_shader_src = R"(#version 300 es


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
    kdtree_search_stack_entry stack[128];

    kdtree_search_stack_entry current;

    current.node_idx = 0u;
    current.has_visited_left = false;
    current.has_visited_right = false;

    while (true)
    {
        if (num_iterations > 256u)
            break;
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

float rand(float n){return fract(sin(n) * 43758.5453123);}

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

    f_color = vec4(
        rand(float(nearest_point_idx + 345u)),
        rand(float(nearest_point_idx + 235u)),
        rand(float(nearest_point_idx + 376u)),
        1
    );
}
)";

std::vector<float> g_points{};
std::vector<uint32_t> g_indices{};
std::vector<kdtree_node> g_kdtree{};

GLuint g_kdtree_buffer{};
GLuint g_kdtree_points_buffer{};

GLuint g_shader_program{};

void generate_points()
{ 
    // Generate points
    g_points.reserve(NUM_POINTS * 2);

    for (uint32_t i = 0; i < NUM_POINTS; i++)
    {
        float p_x = (static_cast<float>(rand()) / static_cast<float>(INT_MAX)) * 1000.0f;
        float p_y = (static_cast<float>(rand()) / static_cast<float>(INT_MAX)) * 1000.0f;

        //printf("Point %.1f, %.1f\n", p_x, p_y);

        g_points.push_back(p_x);
        g_points.push_back(p_y);
    }
    printf("%d points generated\n", NUM_POINTS);

    // Generate indices
    g_indices.resize(NUM_POINTS);
    std::iota(g_indices.begin(), g_indices.end(), 0);

    printf("Indices generated\n");
}

void check_shader_compilation_status(GLuint shader)
{
    GLint status{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint max_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

        std::vector<GLchar> error_log(max_length);
        glGetShaderInfoLog(shader, max_length, &max_length, &error_log[0]);

        //glDeleteShader(shader);

        fprintf(stderr, "Shader failed to compile: %s\n", error_log.data());
        exit(1);
    }
}

void check_program_link_status(GLuint program)
{
    GLint status{};
    glGetProgramiv(program, GL_LINK_STATUS, (int*) &status);

    if (status == GL_FALSE)
    {
        GLint max_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

        std::vector<GLchar> error_log(max_length);
        glGetProgramInfoLog(program, max_length, &max_length, &error_log[0]);
        
        //glDeleteProgram(program);

        fprintf(stderr, "Program failed to compile: %s\n", error_log.data());
        exit(1);
    }
}

void create_shader_program()
{
    GLuint vertex_shader, fragment_shader;
    
    g_shader_program = glCreateProgram();

    char const* src_ptr;

    // Vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &(src_ptr = g_vertex_shader_src.c_str()), nullptr);
    glCompileShader(vertex_shader);
    check_shader_compilation_status(vertex_shader);

    printf("Vertex shader compiled\n");

    glAttachShader(g_shader_program, vertex_shader);

    // Fragment shader
    std::string fragment_shader_src = g_fragment_shader_src;
    fragment_shader_src = std::regex_replace(fragment_shader_src, std::regex("__NUM_KDTREE_NODES__"), std::to_string(NUM_KDTREE_NODES));
    fragment_shader_src = std::regex_replace(fragment_shader_src, std::regex("__NUM_POINTS__"), std::to_string(NUM_POINTS));

    printf(fragment_shader_src.c_str());

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &(src_ptr = fragment_shader_src.c_str()), nullptr);
    glCompileShader(fragment_shader);
    check_shader_compilation_status(fragment_shader);

    printf("Fragment shader compiled\n");

    glAttachShader(g_shader_program, fragment_shader);

    //
    glLinkProgram(g_shader_program);
    check_program_link_status(g_shader_program);
    
    printf("Program linked\n");
}

void rebuild_kdtree()
{
    g_kdtree.clear();
    kdtree_build<2u>(g_points.data(), 2, g_indices.data(), NUM_POINTS, 32, 0, g_kdtree);

    // KD-tree buffer
    glBindBuffer(GL_UNIFORM_BUFFER, g_kdtree_buffer);
    glBufferData(GL_UNIFORM_BUFFER, NUM_KDTREE_NODES * sizeof(kdtree_node), g_kdtree.data(), GL_DYNAMIC_READ);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_kdtree_buffer);

    // KD-tree points buffer
    glBindBuffer(GL_UNIFORM_BUFFER, g_kdtree_points_buffer);
    glBufferData(GL_UNIFORM_BUFFER, NUM_POINTS * (sizeof(float) * 2), g_points.data(), GL_DYNAMIC_READ);

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, g_kdtree_points_buffer);

    //printf("KDTree points - Count: %zu, Buffer size: %lu\n", g_points.size(), NUM_POINTS * (sizeof(float) * 2));
    //printf("KDTree nodes - Count: %zu, Buffer size: %lu\n", g_kdtree.size(), NUM_KDTREE_NODES * sizeof(kdtree_node));

    //printf("KD-tree indices uploaded, Count: %d - Buffer size: %lu\n", g_indices.size(), NUM_POINTS * sizeof(uint32_t));
    //printf("KDtree buffer size: %lu\n", NUM_KDTREE_NODES * sizeof(kdtree_node));

    // Ensure KD-tree and indices are within `MAX_UNIFORM_BLOCK_SIZE`
}

void setup_uniform_buffer_block(char const* block_name, GLuint binding)
{
    GLint block_idx, block_binding, block_size, block_active_uniforms;
    
    block_idx = glGetUniformBlockIndex(g_shader_program, block_name);
    glUniformBlockBinding(g_shader_program, block_idx, binding);

    glGetActiveUniformBlockiv(g_shader_program, block_idx, GL_UNIFORM_BLOCK_BINDING, &block_binding);
    glGetActiveUniformBlockiv(g_shader_program, block_idx, GL_UNIFORM_BLOCK_DATA_SIZE, &block_size);
    glGetActiveUniformBlockiv(g_shader_program, block_idx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &block_active_uniforms);

    printf("%s (index = %d) - Binding: %d, Data size: %d, Active uniforms: %d\n",
        block_name,
        block_idx,
        block_binding,
        block_size,
        block_active_uniforms
    );
}

extern "C" void app_init()
{
    printf("app_init\n");

    srand(time(NULL));

    generate_points();

    g_kdtree.reserve(NUM_KDTREE_NODES);

    create_shader_program();

    // Print WebGL uniform buffer limits
    GLint value{};
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
    printf("GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT: %d\n", value);
    
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
    printf("GL_MAX_UNIFORM_BLOCK_SIZE: %d\n", value);

    // Set uniform blocks binding location
    glUseProgram(g_shader_program);
    
    setup_uniform_buffer_block("KDTreeBlock", 0);
    setup_uniform_buffer_block("KDTreePointsBlock", 1);

    // Create KD-tree buffers
    glGenBuffers(1, &g_kdtree_buffer);
    glGenBuffers(1, &g_kdtree_points_buffer);

    rebuild_kdtree();
}

uint64_t get_current_millis()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

extern "C" void app_draw(uint32_t screen_width, uint32_t screen_height)
{
    static std::optional<uint64_t> last_frame_ms{};
    static uint32_t fps{};
    
    uint64_t now = get_current_millis();
    if (!last_frame_ms || (now - *last_frame_ms) >= 1000)
    {
        printf("FPS: %d\n", fps);

        last_frame_ms = now;
        fps = 0;
    }

    fps++;

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 1);

    glUseProgram(g_shader_program);

    glViewport(0, 0, screen_width, screen_height);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int main(int argc, char* argv[]) { return 0; }
