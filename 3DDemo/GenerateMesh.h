#pragma once

namespace Demo
{

struct Vector2f;
struct Vector3f;

namespace detail
{

Vector3f calculate_quadratic_bezier_vertex(const std::vector<Vector3f>& control_points, float t_u, float t_v);

}

const unsigned int quadratic_bezier_control_point_count = 3;
typedef std::vector<Vector3f> Control_point_patch;

// TODO: 2014: It would make much more sense to do this in a compute shader to generate the data where they are used.
template<typename Output_iterator>
void generate_quadratic_bezier_vertex_array(
    const Control_point_patch& control_points,
    unsigned int patch_count,
    Output_iterator iter)
{
    assert(control_points.size() == (quadratic_bezier_control_point_count * quadratic_bezier_control_point_count));
    const auto surface_vertex_count = patch_count + 1;

    // Generate all of the points.
    for(unsigned int vv = 0; vv < surface_vertex_count; ++vv)
    {
        // Range [0..1].
        const float t_v = vv / static_cast<float>(patch_count);

        for(unsigned int uu = 0; uu < surface_vertex_count; ++uu)
        {
            // Range [0..1].
            const float t_u = uu / static_cast<float>(patch_count);

            *iter++ = detail::calculate_quadratic_bezier_vertex(control_points, t_u, t_v);
        }
    }
}

template<typename Output_iterator>
void generate_implicit_surface_texture_coords_array(unsigned int patch_count, Output_iterator iter)
{
    const auto surface_vertex_count = patch_count + 1;

    const float scale = 1.0f / patch_count;
    for(unsigned int vv = 0; vv < surface_vertex_count; ++vv)
    {
        for(unsigned int uu = 0; uu < surface_vertex_count; ++uu)
        {
            *iter++ = {uu * scale, vv * scale};
        }
    }
}

template<typename Output_iterator>
void generate_implicit_surface_index_array(unsigned int patch_count, size_t bias, Output_iterator iter)
{
    const auto surface_vertex_count = patch_count + 1;

    // Verify that all indices generated will not wrap the max number of vertices.
    assert(bias + patch_count + patch_count * surface_vertex_count <= UINT16_MAX);

    for(unsigned int vv = 0; vv < patch_count; ++vv)
    {
        for(unsigned int uu = 0; uu < patch_count; ++uu)
        {
            *iter++ = static_cast<uint16_t>(bias + (uu + 0) + (vv + 0) * surface_vertex_count);
            *iter++ = static_cast<uint16_t>(bias + (uu + 0) + (vv + 1) * surface_vertex_count);
            *iter++ = static_cast<uint16_t>(bias + (uu + 1) + (vv + 0) * surface_vertex_count);
            *iter++ = static_cast<uint16_t>(bias + (uu + 1) + (vv + 0) * surface_vertex_count);
            *iter++ = static_cast<uint16_t>(bias + (uu + 0) + (vv + 1) * surface_vertex_count);
            *iter++ = static_cast<uint16_t>(bias + (uu + 1) + (vv + 1) * surface_vertex_count);
        }
    }
}

}

