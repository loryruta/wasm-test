#pragma once

#include <cstdint>
#include <cassert>
#include <cmath>
#include <vector>

struct kdtree_node
{
    union {
        float m_split;
        uint32_t m_point_idx;
    };

    uint32_t m_axis; // UINT32_MAX if leaf node

    union {
        uint32_t m_right_node_distance;
        uint32_t m_num_points;
    };

    float _pad;

    bool is_leaf() const
    {
        return m_axis == UINT32_MAX;
    }
};

template<uint32_t _dim>
uint32_t kdtree_build(
    float const* points,
    uint32_t point_stride /* In floats */,
    uint32_t* indices,
    uint32_t count,
    uint32_t max_points_per_division,
    uint32_t node_idx,
    std::vector<kdtree_node>& result
)
{
    assert(point_stride >= _dim);
    assert(count > 0);

    // The maximum number of points per division is reached, we can create a leaf node
    if (count <= max_points_per_division)
    {
        kdtree_node node{};
        node.m_axis = UINT32_MAX; // Leaf node
        node.m_point_idx = indices[0];
        node.m_num_points = count;
        
        result.push_back(node);

        return node_idx + 1;
    }

    // Welfrond's method for computing Variance:
    // https://jonisalonen.com/2013/deriving-welfords-method-for-computing-variance/

    float mean[_dim]{};
    float variance[_dim]{};
    float n = 1.0f;

    for (uint32_t i = 0; i < count; i++, n += 1.0f)
    {
        float const* point = &points[indices[i] * point_stride];

        for (uint32_t j = 0; j < _dim; j++)
        {
            float d = point[j] - mean[j];
            mean[j] += d / n;
            variance[j] += (point[j] - mean[j]) * d;
        }
    }

    // Find the axis that has the highest Variance and the split value is its Mean
    float highest_variance = -std::numeric_limits<float>::infinity();
    int axis = -1;
    for (uint32_t j = 0; j < _dim; j++)
    {
        if (highest_variance < variance[j])
        {
            highest_variance = variance[j];
            axis = j;
        }
    }
    float split = mean[axis];

    // Put the points that are on the left of the split on the left of the array
    uint32_t boundary = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        float const* point = &points[indices[i] * point_stride];

        if (point[axis] < split)
        {
            std::swap(indices[i], indices[boundary]);
            boundary++;
        }
    }
    
    uint32_t next_node_idx{};

    // Left side
    next_node_idx = kdtree_build<_dim>(
        points,
        point_stride,
        indices,
        boundary,
        max_points_per_division,
        node_idx + 1,
        result
    );

    kdtree_node node{};
    node.m_split = split;
    node.m_axis = axis;
    node.m_right_node_distance = next_node_idx - node_idx;

    result.push_back(node);

    // Right side
    next_node_idx = kdtree_build<_dim>(
        points,
        point_stride,
        boundary + indices,
        count - boundary,
        max_points_per_division,
        next_node_idx,
        result
    );

    return next_node_idx;
}

template<uint32_t _dim>
void kdtree_search_rec(
    float const* points,
    uint32_t point_stride /* In floats */,
    uint32_t const* indices,
    kdtree_node const* current_node,
    float const* sample_point,
    uint32_t* nearest_point_idx,
    float* nearest_distance_squared
)
{
    if (current_node->is_leaf())
    {
        for (uint32_t i = 0; i < current_node->m_num_points; i++)
        {
            uint32_t point_idx = indices[current_node->m_point_idx + i];
            float const* point = &points[point_idx * point_stride];

            float distance = 0.0f;
            for (uint32_t j = 0; j < _dim; j++)
                distance += (point[j] - sample_point[j]) * (point[j] - sample_point[j]);

            if (distance < *nearest_distance_squared)
            {
                *nearest_point_idx = point_idx;
                *nearest_distance_squared = distance;
            }
        }
    }
    else
    {
        kdtree_node const* left_node = current_node + 1;
        kdtree_node const* right_node = current_node + current_node->m_right_node_distance;

        float signed_distance = sample_point[current_node->m_axis] - current_node->m_split;
        float signed_distance_squared = signed_distance * signed_distance;

        // Give priority to the split to which the sample belongs to since it's more likely to find neighbors there
        kdtree_search_rec<_dim>(points, point_stride, indices, signed_distance <= 0 ? left_node : right_node, sample_point, nearest_point_idx, nearest_distance_squared);

        // Then search in the other split only if the distance to the split is less than the nearest distance already found
        if (abs(signed_distance_squared) < *nearest_distance_squared)
             kdtree_search_rec<_dim>(points, point_stride, indices, signed_distance <= 0 ? right_node : left_node, sample_point, nearest_point_idx, nearest_distance_squared);
    }
}

template<uint32_t _dim>
void kdtree_search(
    float const* points,
    uint32_t point_stride /* In floats */,
    uint32_t const* indices,
    kdtree_node const* kdtree,
    float const* sample_point,
    uint32_t* nearest_point,
    float* nearest_distance_squared
)
{
    assert(nearest_point != nullptr);
    assert(nearest_distance_squared != nullptr);

    *nearest_point = UINT32_MAX;
    *nearest_distance_squared = std::numeric_limits<float>::infinity();

    kdtree_search_rec<_dim>(
        points,
        point_stride,
        indices,
        kdtree,
        sample_point,
        nearest_point,
        nearest_distance_squared
    );
}
