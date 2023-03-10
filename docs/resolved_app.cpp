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
#define BOUNCING_ENABLED

char const* g_screen_quad_shader_src = "\x23\x76\x65\x72\x73\x69\x6f\x6e\x20\x33\x30\x30\x20\x65\x73\xd\xa\xd\xa\x70\x72\x65\x63\x69\x73\x69\x6f\x6e\x20\x68\x69\x67\x68\x70\x20\x66\x6c\x6f\x61\x74\x3b\xd\xa\xd\xa\x76\x6f\x69\x64\x20\x6d\x61\x69\x6e\x28\x29\xd\xa\x7b\xd\xa\x20\x20\x20\x20\x76\x65\x63\x32\x20\x73\x63\x72\x65\x65\x6e\x5f\x71\x75\x61\x64\x5b\x36\x5d\x20\x3d\x20\x76\x65\x63\x32\x5b\x5d\x28\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x76\x65\x63\x32\x28\x2d\x31\x2e\x30\x2c\x20\x2d\x31\x2e\x30\x29\x2c\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x76\x65\x63\x32\x28\x2d\x31\x2e\x30\x2c\x20\x31\x2e\x30\x29\x2c\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x76\x65\x63\x32\x28\x31\x2e\x30\x2c\x20\x31\x2e\x30\x29\x2c\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x76\x65\x63\x32\x28\x31\x2e\x30\x2c\x20\x31\x2e\x30\x29\x2c\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x76\x65\x63\x32\x28\x31\x2e\x30\x2c\x20\x2d\x31\x2e\x30\x29\x2c\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x76\x65\x63\x32\x28\x2d\x31\x2e\x30\x2c\x20\x2d\x31\x2e\x30\x29\xd\xa\x20\x20\x20\x20\x29\x3b\xd\xa\x20\x20\x20\x20\x67\x6c\x5f\x50\x6f\x73\x69\x74\x69\x6f\x6e\x20\x3d\x20\x76\x65\x63\x34\x28\x73\x63\x72\x65\x65\x6e\x5f\x71\x75\x61\x64\x5b\x67\x6c\x5f\x56\x65\x72\x74\x65\x78\x49\x44\x5d\x2c\x20\x30\x2c\x20\x31\x29\x3b\xd\xa\x7d\xd\xa";
char const* g_bounce_points_shader_src = "\x23\x76\x65\x72\x73\x69\x6f\x6e\x20\x33\x30\x30\x20\x65\x73\xd\xa\xd\xa\x70\x72\x65\x63\x69\x73\x69\x6f\x6e\x20\x68\x69\x67\x68\x70\x20\x66\x6c\x6f\x61\x74\x3b\xd\xa\xd\xa\x75\x6e\x69\x66\x6f\x72\x6d\x20\x73\x61\x6d\x70\x6c\x65\x72\x32\x44\x20\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x3b\xd\xa\x75\x6e\x69\x66\x6f\x72\x6d\x20\x73\x61\x6d\x70\x6c\x65\x72\x32\x44\x20\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x3b\xd\xa\xd\xa\x75\x6e\x69\x66\x6f\x72\x6d\x20\x66\x6c\x6f\x61\x74\x20\x75\x5f\x64\x74\x3b\xd\xa\x75\x6e\x69\x66\x6f\x72\x6d\x20\x76\x65\x63\x32\x20\x75\x5f\x73\x63\x72\x65\x65\x6e\x5f\x73\x69\x7a\x65\x3b\xd\xa\xd\xa\x6c\x61\x79\x6f\x75\x74\x28\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x20\x3d\x20\x30\x29\x20\x6f\x75\x74\x20\x76\x65\x63\x32\x20\x6f\x75\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x3b\xd\xa\x6c\x61\x79\x6f\x75\x74\x28\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x20\x3d\x20\x31\x29\x20\x6f\x75\x74\x20\x76\x65\x63\x32\x20\x6f\x75\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x3b\xd\xa\xd\xa\x76\x6f\x69\x64\x20\x75\x70\x64\x61\x74\x65\x5f\x70\x6f\x69\x6e\x74\x28\xd\xa\x20\x20\x20\x20\x69\x6e\x6f\x75\x74\x20\x76\x65\x63\x32\x20\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2c\xd\xa\x20\x20\x20\x20\x69\x6e\x6f\x75\x74\x20\x76\x65\x63\x32\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\xd\xa\x29\xd\xa\x7b\xd\xa\x20\x20\x20\x20\x63\x6f\x6e\x73\x74\x20\x66\x6c\x6f\x61\x74\x20\x6b\x5f\x73\x70\x65\x65\x64\x20\x3d\x20\x32\x2e\x30\x3b\xd\xa\x20\x20\x20\x20\x63\x6f\x6e\x73\x74\x20\x66\x6c\x6f\x61\x74\x20\x6b\x5f\x6d\x61\x72\x67\x69\x6e\x20\x3d\x20\x31\x2e\x30\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x20\x2b\x3d\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x20\x2a\x20\x6b\x5f\x73\x70\x65\x65\x64\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x69\x66\x20\x28\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2e\x78\x20\x3c\x20\x30\x2e\x30\x29\xd\xa\x20\x20\x20\x20\x7b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2e\x78\x20\x3d\x20\x2d\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2e\x78\x3b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2e\x78\x20\x3d\x20\x6b\x5f\x6d\x61\x72\x67\x69\x6e\x3b\xd\xa\x20\x20\x20\x20\x7d\xd\xa\xd\xa\x20\x20\x20\x20\x69\x66\x20\x28\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2e\x78\x20\x3e\x20\x75\x5f\x73\x63\x72\x65\x65\x6e\x5f\x73\x69\x7a\x65\x2e\x78\x29\xd\xa\x20\x20\x20\x20\x7b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2e\x78\x20\x3d\x20\x2d\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2e\x78\x3b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2e\x78\x20\x3d\x20\x75\x5f\x73\x63\x72\x65\x65\x6e\x5f\x73\x69\x7a\x65\x2e\x78\x20\x2d\x20\x6b\x5f\x6d\x61\x72\x67\x69\x6e\x3b\xd\xa\x20\x20\x20\x20\x7d\xd\xa\xd\xa\x20\x20\x20\x20\x69\x66\x20\x28\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2e\x79\x20\x3c\x20\x30\x2e\x30\x29\xd\xa\x20\x20\x20\x20\x7b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2e\x79\x20\x3d\x20\x2d\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2e\x79\x3b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2e\x79\x20\x3d\x20\x6b\x5f\x6d\x61\x72\x67\x69\x6e\x3b\xd\xa\x20\x20\x20\x20\x7d\xd\xa\xd\xa\x20\x20\x20\x20\x69\x66\x20\x28\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2e\x79\x20\x3e\x20\x75\x5f\x73\x63\x72\x65\x65\x6e\x5f\x73\x69\x7a\x65\x2e\x79\x29\xd\xa\x20\x20\x20\x20\x7b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2e\x79\x20\x3d\x20\x2d\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2e\x79\x3b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2e\x79\x20\x3d\x20\x75\x5f\x73\x63\x72\x65\x65\x6e\x5f\x73\x69\x7a\x65\x2e\x79\x20\x2d\x20\x6b\x5f\x6d\x61\x72\x67\x69\x6e\x3b\xd\xa\x20\x20\x20\x20\x7d\xd\xa\x7d\xd\xa\xd\xa\x76\x6f\x69\x64\x20\x6d\x61\x69\x6e\x28\x29\xd\xa\x7b\xd\xa\x20\x20\x20\x20\x76\x65\x63\x32\x20\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x20\x3d\x20\x74\x65\x78\x65\x6c\x46\x65\x74\x63\x68\x28\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2c\x20\x69\x76\x65\x63\x32\x28\x67\x6c\x5f\x46\x72\x61\x67\x43\x6f\x6f\x72\x64\x2e\x78\x79\x29\x2c\x20\x30\x29\x2e\x72\x67\x3b\xd\xa\x20\x20\x20\x20\x76\x65\x63\x32\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x20\x3d\x20\x74\x65\x78\x65\x6c\x46\x65\x74\x63\x68\x28\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x2c\x20\x69\x76\x65\x63\x32\x28\x67\x6c\x5f\x46\x72\x61\x67\x43\x6f\x6f\x72\x64\x2e\x78\x79\x29\x2c\x20\x30\x29\x2e\x72\x67\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x75\x70\x64\x61\x74\x65\x5f\x70\x6f\x69\x6e\x74\x28\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2c\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x29\x3b\xd\xa\x20\xd\xa\x20\x20\x20\x20\x6f\x75\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x20\x3d\x20\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x3b\xd\xa\x20\x20\x20\x20\x6f\x75\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x20\x3d\x20\x70\x6f\x69\x6e\x74\x5f\x64\x69\x72\x3b\xd\xa\x7d\xd\xa";
char const* g_voronoi_shader_src = "\x23\x76\x65\x72\x73\x69\x6f\x6e\x20\x33\x30\x30\x20\x65\x73\xd\xa\xd\xa\x70\x72\x65\x63\x69\x73\x69\x6f\x6e\x20\x68\x69\x67\x68\x70\x20\x66\x6c\x6f\x61\x74\x3b\xd\xa\xd\xa\x23\x64\x65\x66\x69\x6e\x65\x20\x49\x4e\x46\x20\x20\x20\x20\x20\x20\x20\x20\x31\x65\x33\x35\x3b\xd\xa\x23\x64\x65\x66\x69\x6e\x65\x20\x55\x49\x4e\x54\x33\x32\x5f\x4d\x41\x58\x20\x30\x78\x46\x46\x46\x46\x46\x46\x46\x46\x75\xd\xa\xd\xa\x75\x6e\x69\x66\x6f\x72\x6d\x20\x73\x61\x6d\x70\x6c\x65\x72\x32\x44\x20\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x3b\xd\xa\xd\xa\x6f\x75\x74\x20\x76\x65\x63\x34\x20\x66\x72\x61\x67\x5f\x63\x6f\x6c\x6f\x72\x3b\xd\xa\xd\xa\x66\x6c\x6f\x61\x74\x20\x72\x61\x6e\x64\x28\x66\x6c\x6f\x61\x74\x20\x6e\x29\x7b\x72\x65\x74\x75\x72\x6e\x20\x66\x72\x61\x63\x74\x28\x73\x69\x6e\x28\x6e\x29\x20\x2a\x20\x34\x33\x37\x35\x38\x2e\x35\x34\x35\x33\x31\x32\x33\x29\x3b\x7d\xd\xa\xd\xa\x76\x6f\x69\x64\x20\x6d\x61\x69\x6e\x28\x29\xd\xa\x7b\xd\xa\x20\x20\x20\x20\x76\x65\x63\x32\x20\x6d\x79\x5f\x70\x6f\x69\x6e\x74\x20\x3d\x20\x67\x6c\x5f\x46\x72\x61\x67\x43\x6f\x6f\x72\x64\x2e\x78\x79\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x69\x76\x65\x63\x32\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x69\x64\x78\x5b\x32\x5d\x3b\xd\xa\x20\x20\x20\x20\x66\x6c\x6f\x61\x74\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x32\x5d\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x30\x5d\x20\x3d\x20\x49\x4e\x46\x3b\xd\xa\x20\x20\x20\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x31\x5d\x20\x3d\x20\x49\x4e\x46\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x69\x76\x65\x63\x32\x20\x74\x65\x78\x74\x75\x72\x65\x5f\x73\x69\x7a\x65\x20\x3d\x20\x74\x65\x78\x74\x75\x72\x65\x53\x69\x7a\x65\x28\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2c\x20\x30\x29\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x66\x6f\x72\x20\x28\x69\x6e\x74\x20\x78\x20\x3d\x20\x30\x3b\x20\x78\x20\x3c\x20\x74\x65\x78\x74\x75\x72\x65\x5f\x73\x69\x7a\x65\x2e\x78\x3b\x20\x78\x2b\x2b\x29\xd\xa\x20\x20\x20\x20\x7b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6f\x72\x20\x28\x69\x6e\x74\x20\x79\x20\x3d\x20\x30\x3b\x20\x79\x20\x3c\x20\x74\x65\x78\x74\x75\x72\x65\x5f\x73\x69\x7a\x65\x2e\x79\x3b\x20\x79\x2b\x2b\x29\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x7b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x76\x65\x63\x32\x20\x70\x6f\x69\x6e\x74\x20\x3d\x20\x74\x65\x78\x65\x6c\x46\x65\x74\x63\x68\x28\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2c\x20\x69\x76\x65\x63\x32\x28\x78\x2c\x20\x79\x29\x2c\x20\x30\x29\x2e\x72\x67\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x66\x6c\x6f\x61\x74\x20\x64\x20\x3d\x20\x64\x69\x73\x74\x61\x6e\x63\x65\x28\x70\x6f\x69\x6e\x74\x2c\x20\x6d\x79\x5f\x70\x6f\x69\x6e\x74\x29\x3b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x20\x28\x64\x20\x3c\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x30\x5d\x29\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x7b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x31\x5d\x20\x3d\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x30\x5d\x3b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x69\x64\x78\x5b\x31\x5d\x20\x3d\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x69\x64\x78\x5b\x30\x5d\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x30\x5d\x20\x3d\x20\x64\x3b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x69\x64\x78\x5b\x30\x5d\x20\x3d\x20\x69\x76\x65\x63\x32\x28\x78\x2c\x20\x79\x29\x3b\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x7d\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x7d\xd\xa\x20\x20\x20\x20\x7d\xd\xa\xd\xa\x20\x20\x20\x20\x69\x6e\x74\x20\x70\x69\x20\x3d\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x69\x64\x78\x5b\x30\x5d\x2e\x78\x20\x2a\x20\x74\x65\x78\x74\x75\x72\x65\x5f\x73\x69\x7a\x65\x2e\x79\x20\x2b\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x69\x64\x78\x5b\x30\x5d\x2e\x79\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x76\x65\x63\x33\x20\x70\x6f\x69\x6e\x74\x5f\x63\x6f\x6c\x6f\x72\x20\x3d\x20\x76\x65\x63\x33\x28\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x6e\x64\x28\x66\x6c\x6f\x61\x74\x28\x70\x69\x20\x2b\x20\x35\x36\x37\x38\x29\x29\x20\x2a\x20\x30\x2e\x36\x20\x2b\x20\x30\x2e\x32\x2c\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x6e\x64\x28\x66\x6c\x6f\x61\x74\x28\x70\x69\x20\x2b\x20\x32\x33\x34\x32\x29\x29\x20\x2a\x20\x30\x2e\x36\x20\x2b\x20\x30\x2e\x32\x2c\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x72\x61\x6e\x64\x28\x66\x6c\x6f\x61\x74\x28\x70\x69\x20\x2b\x20\x38\x39\x30\x35\x29\x29\x20\x2a\x20\x30\x2e\x36\x20\x2b\x20\x30\x2e\x32\xd\xa\x20\x20\x20\x20\x29\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x63\x6f\x6e\x73\x74\x20\x66\x6c\x6f\x61\x74\x20\x6b\x5f\x62\x6f\x72\x64\x65\x72\x5f\x73\x69\x7a\x65\x20\x3d\x20\x31\x32\x33\x2e\x30\x3b\xd\xa\x20\x20\x20\x20\x63\x6f\x6e\x73\x74\x20\x66\x6c\x6f\x61\x74\x20\x6b\x5f\x62\x6f\x72\x64\x65\x72\x5f\x73\x68\x61\x64\x65\x5f\x73\x74\x72\x69\x70\x65\x73\x20\x3d\x20\x33\x2e\x38\x3b\xd\xa\x20\x20\x20\x20\x63\x6f\x6e\x73\x74\x20\x76\x65\x63\x33\x20\x6b\x5f\x62\x6f\x72\x64\x65\x72\x5f\x63\x6f\x6c\x6f\x72\x20\x3d\x20\x76\x65\x63\x33\x28\x30\x2c\x20\x30\x2c\x20\x30\x2e\x32\x29\x3b\x20\xd\xa\xd\xa\x20\x20\x20\x20\x76\x65\x63\x32\x20\x70\x31\x20\x3d\x20\x74\x65\x78\x65\x6c\x46\x65\x74\x63\x68\x28\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2c\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x69\x64\x78\x5b\x30\x5d\x2c\x20\x30\x29\x2e\x72\x67\x3b\xd\xa\x20\x20\x20\x20\x76\x65\x63\x32\x20\x70\x32\x20\x3d\x20\x74\x65\x78\x65\x6c\x46\x65\x74\x63\x68\x28\x75\x5f\x70\x6f\x69\x6e\x74\x5f\x70\x6f\x73\x2c\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x69\x64\x78\x5b\x31\x5d\x2c\x20\x31\x29\x2e\x72\x67\x3b\xd\xa\x20\x20\x20\x20\x76\x65\x63\x32\x20\x6d\x64\x20\x3d\x20\x28\x70\x31\x20\x2b\x20\x70\x32\x29\x20\x2f\x20\x32\x2e\x30\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x66\x6c\x6f\x61\x74\x20\x64\x31\x20\x3d\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x30\x5d\x3b\xd\xa\x20\x20\x20\x20\x66\x6c\x6f\x61\x74\x20\x64\x32\x20\x3d\x20\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x31\x5d\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x66\x6c\x6f\x61\x74\x20\x65\x64\x20\x3d\x20\x61\x62\x73\x28\x64\x31\x20\x2d\x20\x64\x32\x29\x3b\xd\xa\x20\x20\x20\x20\x2f\x2f\x66\x6c\x6f\x61\x74\x20\x63\x64\x20\x3d\x20\x61\x62\x73\x28\x28\x70\x32\x2e\x78\x20\x2d\x20\x70\x31\x2e\x78\x29\x20\x2a\x20\x28\x70\x31\x2e\x79\x20\x2d\x20\x6d\x79\x5f\x70\x6f\x69\x6e\x74\x2e\x79\x29\x20\x2d\x20\x28\x70\x31\x2e\x78\x20\x2d\x20\x6d\x79\x5f\x70\x6f\x69\x6e\x74\x2e\x78\x29\x20\x2a\x20\x28\x70\x32\x2e\x79\x20\x2d\x20\x70\x31\x2e\x79\x29\x29\x20\x2f\x20\x64\x69\x73\x74\x61\x6e\x63\x65\x28\x70\x32\x2c\x20\x70\x31\x29\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x66\x6c\x6f\x61\x74\x20\x66\x64\x20\x3d\x20\x65\x64\x3b\xd\xa\x20\x20\x20\x20\x66\x64\x20\x3d\x20\x6d\x69\x6e\x28\x66\x64\x2c\x20\x6b\x5f\x62\x6f\x72\x64\x65\x72\x5f\x73\x69\x7a\x65\x29\x20\x2f\x20\x6b\x5f\x62\x6f\x72\x64\x65\x72\x5f\x73\x69\x7a\x65\x3b\xd\xa\x20\x20\x20\x20\x66\x64\x20\x3d\x20\x66\x64\x20\x2d\x20\x6d\x6f\x64\x28\x66\x64\x2c\x20\x28\x31\x2e\x30\x20\x2f\x20\x6b\x5f\x62\x6f\x72\x64\x65\x72\x5f\x73\x68\x61\x64\x65\x5f\x73\x74\x72\x69\x70\x65\x73\x29\x29\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x2f\x2f\x20\x53\x48\x4f\x57\x20\x56\x4f\x52\x4f\x4e\x4f\x49\xd\xa\x20\x20\x20\x20\x66\x72\x61\x67\x5f\x63\x6f\x6c\x6f\x72\x20\x3d\x20\x76\x65\x63\x34\x28\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x6d\x69\x78\x28\x6b\x5f\x62\x6f\x72\x64\x65\x72\x5f\x63\x6f\x6c\x6f\x72\x2c\x20\x70\x6f\x69\x6e\x74\x5f\x63\x6f\x6c\x6f\x72\x2c\x20\x66\x64\x29\x2c\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x31\xd\xa\x20\x20\x20\x20\x29\x3b\xd\xa\xd\xa\x20\x20\x20\x20\x2f\x2a\xd\xa\x20\x20\x20\x20\x2f\x2f\x20\x53\x48\x4f\x57\x20\x50\x4f\x49\x4e\x54\x53\xd\xa\x20\x20\x20\x20\x69\x66\x20\x28\x6e\x65\x61\x72\x65\x73\x74\x5f\x70\x6f\x69\x6e\x74\x5f\x64\x69\x73\x74\x61\x6e\x63\x65\x5b\x30\x5d\x20\x3c\x20\x31\x35\x2e\x30\x29\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x66\x72\x61\x67\x5f\x63\x6f\x6c\x6f\x72\x20\x3d\x20\x76\x65\x63\x34\x28\x30\x2c\x20\x30\x2c\x20\x30\x2c\x20\x31\x29\x3b\xd\xa\x20\x20\x20\x20\x65\x6c\x73\x65\xd\xa\x20\x20\x20\x20\x20\x20\x20\x20\x66\x72\x61\x67\x5f\x63\x6f\x6c\x6f\x72\x20\x3d\x20\x76\x65\x63\x34\x28\x70\x6f\x69\x6e\x74\x5f\x63\x6f\x6c\x6f\x72\x2c\x20\x31\x29\x3b\x2a\x2f\xd\xa\x7d\xd\xa";

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

    //srand((uint32_t) get_current_millis());

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
#ifdef BOUNCING_ENABLED
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
#endif

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
