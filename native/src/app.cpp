#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <numeric>
#include <string>
#include <regex>
#include <optional>
#include <random>

#include <GLES3/gl3.h>

#define NUM_POINTS_X 6
#define NUM_POINTS_Y 5

char const* g_screen_quad_shader_src = "{{ SCREEN_QUAD_SRC }}";
char const* g_bounce_points_shader_src = "{{ BOUNCE_POINTS_SRC }}";
char const* g_voronoi_shader_src = "{{ VORONOI_SRC }}";

GLuint g_bounce_points_program{};
GLuint g_voronoi_program{};

GLuint g_bounce_points_framebuffer{};

GLuint g_point_positions_textures[2];
GLuint g_point_directions_textures[2];

uint32_t g_frame_idx = 0;

float rand_float()
{
    return ((float) rand()) / ((float) RAND_MAX);
}

uint64_t get_current_millis()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void initialize_point_textures()
{
    std::vector<float> point_positions{};
    std::vector<float> point_directions{};

    for (int i = 0; i < NUM_POINTS_X * NUM_POINTS_Y; i++)
    {
        float pos_x = rand_float() * 1024.0f;
        float pos_y = rand_float() * 1024.0f;

        point_positions.push_back(pos_x);
        point_positions.push_back(pos_y);

        float dir_x = rand_float() * 2.0f - 1.0f;
        float dir_y = rand_float() * 2.0f - 1.0f;

        float dir_norm = sqrt(dir_x * dir_x + dir_y * dir_y);
        
        dir_x /= dir_norm;
        dir_y /= dir_norm;
        
        printf("Point %.1f, %.1f - Direction %.3f, %.3f\n",
            pos_x, pos_y,
            dir_x, dir_y);

        point_directions.push_back(dir_x);
        point_directions.push_back(dir_y);
    }

    for (int i = 0; i < 2; i++)
    {
        // Create point positions texture
        glGenTextures(1, &g_point_positions_textures[i]);

        glBindTexture(GL_TEXTURE_2D, g_point_positions_textures[i]);

        // The image data should only be uploaded for the first buffer since the second buffer values will be initialized by the
        // bounce_lights pass. However since in WebGL we don't have correct synchronization between writes/reads from the same texture
        // in two subsequent passes, it's practically resulted that initializing both buffers with the same data works
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, NUM_POINTS_X, NUM_POINTS_Y, 0, GL_RG, GL_FLOAT, point_positions.data());
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Create point directions texture
        glGenTextures(1, &g_point_directions_textures[i]);
        
        glBindTexture(GL_TEXTURE_2D, g_point_directions_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, NUM_POINTS_X, NUM_POINTS_Y, 0, GL_RG, GL_FLOAT, point_directions.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

GLuint create_shader(GLuint type, char const* src)
{
    GLuint shader;
    
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    return shader;
}

GLint get_uniform_location(GLuint program, char const* uniform_name)
{
    GLint loc = glGetUniformLocation(program, uniform_name);
    if (loc < 0)
    {
        fprintf(stderr, "Invalid uniform name: %s\n", uniform_name);
        exit(1);
    }

    return loc;
}

void create_bounce_points_program()
{
    GLuint screen_quad = create_shader(GL_VERTEX_SHADER, g_screen_quad_shader_src);
    GLuint bounce_points = create_shader(GL_FRAGMENT_SHADER, g_bounce_points_shader_src);

    g_bounce_points_program = glCreateProgram();

    glAttachShader(g_bounce_points_program, screen_quad);
    glAttachShader(g_bounce_points_program, bounce_points);

    glLinkProgram(g_bounce_points_program);

    glDeleteShader(screen_quad);
    glDeleteShader(bounce_points);

    // Set texture units
    glUseProgram(g_bounce_points_program);

    glUniform1i(get_uniform_location(g_bounce_points_program, "u_point_pos"), 0);
    glUniform1i(get_uniform_location(g_bounce_points_program, "u_point_dir"), 1);
}

void create_voronoi_program()
{
    GLuint screen_quad = create_shader(GL_VERTEX_SHADER, g_screen_quad_shader_src);
    GLuint voronoi = create_shader(GL_FRAGMENT_SHADER, g_voronoi_shader_src);

    g_voronoi_program = glCreateProgram();

    glAttachShader(g_voronoi_program, screen_quad);
    glAttachShader(g_voronoi_program, voronoi);

    glLinkProgram(g_voronoi_program);
    
    glDeleteShader(screen_quad);
    glDeleteShader(voronoi);
    
    // Set texture units
    glUseProgram(g_voronoi_program);

    glUniform1i(get_uniform_location(g_voronoi_program, "u_point_pos"), 0);
}

extern "C" void app_init()
{
    printf("app_init\n");

    srand((uint32_t) get_current_millis());

    initialize_point_textures();
    
    printf("Creating bounce_points program\n");
    create_bounce_points_program();

    printf("Creating voronoi program\n");
    create_voronoi_program();

    glGenFramebuffers(1, &g_bounce_points_framebuffer);
}

extern "C" void app_draw(uint32_t screen_width, uint32_t screen_height)
{
    static std::optional<uint64_t> last_frame_ms{};
    static uint32_t fps{};
    
    uint64_t now = get_current_millis();
    float dt = last_frame_ms ? static_cast<float>(now - *last_frame_ms) / 1000.0f : 0.0f;
    if (!last_frame_ms || dt >= 1.0f)
    {
        printf("FPS: %d\n", fps);

        last_frame_ms = now;
        fps = 0;
    }

    fps++;


    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 1);

    /* Bounce points */
    if (dt > 0)
    {
        glUseProgram(g_bounce_points_program);

        glViewport(0, 0, NUM_POINTS_X, NUM_POINTS_Y);

        // Setup & bind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, g_bounce_points_framebuffer);
        
        GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, draw_buffers);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_point_positions_textures[(g_frame_idx + 1) % 2], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_point_directions_textures[(g_frame_idx + 1) % 2], 0);

        // Bind uniforms
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_point_positions_textures[g_frame_idx % 2]);  // u_point_pos

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_point_directions_textures[g_frame_idx % 2]); // u_point_dir

        //glUniform1f(get_uniform_location(g_bounce_points_program, "u_dt"), dt);
        glUniform2f(get_uniform_location(g_bounce_points_program, "u_screen_size"), (float) screen_width, (float) screen_height);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    } 

    /* Voronoi */
    glUseProgram(g_voronoi_program);

    glViewport(0, 0, screen_width, screen_height);

    glBindFramebuffer(GL_FRAMEBUFFER, /* Default */ 0);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, g_point_positions_textures[0]);//(g_frame_idx + 1) % 2]); // u_point_pos

    glDrawArrays(GL_TRIANGLES, 0, 6);

    //
    g_frame_idx++;
}

int main(int argc, char* argv[]) { return 0; }
