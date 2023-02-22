#include <cstdio>
#include <vector>
#include <numeric>
#include <chrono>

#include "kdtree.hpp"

void generate_point(float point[3])
{
    for (int i = 0; i < 3; i++)
        point[i] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 100.0f;
}

uint32_t find_nearest_point(float const* points, uint32_t n, float const* sample_point)
{
    float min_distance = std::numeric_limits<float>::infinity();
    uint32_t min_point_idx = UINT32_MAX;

    for (uint32_t i = 0; i < n; i++)
    {
        float const* point = &points[i * 3];
        
        float distance = 0;
        for (uint32_t j = 0; j < 3; j++)
            distance += (point[j] - sample_point[j]) * (point[j] - sample_point[j]);

        if (distance < min_distance)
        {
            min_distance = distance;
            min_point_idx = i;
        }
    }
    return min_point_idx;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));

    const uint32_t N = 10'000;

    printf("Generating %d points...\n", N);

    std::vector<float> points{};
    for (uint32_t i = 0; i < N; i++)
    {
        float point[3];
        generate_point(point);

        points.push_back(point[0]);
        points.push_back(point[1]);
        points.push_back(point[2]);
    }

    printf("%d points generated\n", N);

    std::vector<uint32_t> indices(N);
    std::iota(indices.begin(), indices.end(), 0);

    printf("Building kd-tree...\n");

    std::vector<kdtree_node> kdtree{};
    {
        auto started_at = std::chrono::steady_clock::now();

        kdtree_build<3u>(points.data(), 3, indices.data(), N, 8, 0, kdtree);
        
        auto ended_at = std::chrono::steady_clock::now();

        printf("Built kd-tree of %d nodes in %lldus\n",
            kdtree.size(),    
            std::chrono::duration_cast<std::chrono::microseconds>(ended_at - started_at).count()
        );
    }

    float sample_point[3];
    generate_point(sample_point);

    printf("Sample point is at (%.1f, %.1f, %.1f)\n", sample_point[0], sample_point[1], sample_point[2]);

    // Find the nearest point with a linear search O(N)
    uint32_t nearest_point_1;
    {
        auto started_at = std::chrono::steady_clock::now();

        nearest_point_1 = find_nearest_point(points.data(), N, sample_point);

        auto ended_at = std::chrono::steady_clock::now();

        printf("Linear search: Nearest point is %d at (%.1f, %.1f, %.1f), found in %lldus\n",
            nearest_point_1,
            points[nearest_point_1 * 3],
            points[nearest_point_1 * 3 + 1],
            points[nearest_point_1 * 3 + 2],
            std::chrono::duration_cast<std::chrono::microseconds>(ended_at - started_at).count()
        );
    }

    // Find the nearest point through the KD-tree structure
    uint32_t nearest_point_2;
    {
        auto started_at = std::chrono::steady_clock::now();

        float nearest_distance_squared;
        kdtree_search<3u>(points.data(), 3, indices.data(), kdtree.data(), sample_point, &nearest_point_2, &nearest_distance_squared);

        auto ended_at = std::chrono::steady_clock::now();

        printf("KD-tree search: Nearest point is %d at (%.1f, %.1f, %.1f) (distance=%f), found in %lldus\n",
            nearest_point_1,
            points[nearest_point_1 * 3],
            points[nearest_point_1 * 3 + 1],
            points[nearest_point_1 * 3 + 2],
            sqrt(nearest_distance_squared),
            std::chrono::duration_cast<std::chrono::microseconds>(ended_at - started_at).count()
        );
    }

    return 0;
}
