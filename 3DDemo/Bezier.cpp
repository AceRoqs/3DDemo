#include "PreCompile.h"
#include "Bezier.h"
#include "LinearAlgebra.h"

namespace Demo
{

// http://en.wikipedia.org/wiki/B%C3%A9zier_curve
// B0(t) = (1-t)^2
// B1(t) = 2(1-t)t
// B2(t) = t^2
// t = [0..1]
static float bezier_quadratic_basis(unsigned int index, float t)
{
    float basis;
    if(index == 0)
    {
        basis = (1.0f - t) * (1.0f - t);
    }
    else if(index == 1)
    {
        basis = 2.0f * (1.0f - t) * t;
    }
    else
    {
        assert(index == 2);
        basis = t * t;
    }

    return basis;
}

static Vector3f calculate_quadratic_bezier_vertex(const std::vector<struct Vector3f>& control_points, float t_u, float t_v)
{
    Vector3f vertex = make_vector(0.0f, 0.0f, 0.0f);

    // Calculate the u,v point of the patch using three control points in each (u/v) direction.
    for(unsigned int j = 0; j < quadratic_bezier_control_point_count; ++j)
    {
        const float basis_v = bezier_quadratic_basis(j, t_v);

        for(unsigned int i = 0; i < quadratic_bezier_control_point_count; ++i)
        {
            const float basis = bezier_quadratic_basis(i, t_u) * basis_v;

            // Q(u,v) = sum[i=0..2]sum[j=0..2] Bi(u)Bj(v)Pij
            vertex += control_points[j * quadratic_bezier_control_point_count + i] * basis;
        }
    }

    return vertex;
}

// TODO: 2014: It would make much more sense to do this in a compute shader to generate the data where they are used.
void append_quadratic_bezier_vertex_patch(const Control_point_patch& control_points, unsigned int patch_count, std::vector<Vector3f>& vertices)
{
    const auto curve_vertex_count = patch_count + 1;

    vertices.reserve(vertices.size() + curve_vertex_count * curve_vertex_count);

    // Generate all of the points.
    for(unsigned int v = 0; v < curve_vertex_count; ++v)
    {
        // Range [0..1].
        const float t_v = v / static_cast<float>(patch_count);

        for(unsigned int u = 0; u < curve_vertex_count; ++u)
        {
            // Range [0..1].
            const float t_u = u / static_cast<float>(patch_count);

            vertices.push_back(calculate_quadratic_bezier_vertex(control_points, t_u, t_v));
        }
    }
}

// TODO: 2014: It would make much more sense to do this in a compute shader to generate the data where they are used.
void generate_patch_quadratic_bezier_vertex_array(
    const Control_point_patch& control_points,
    unsigned int patch_count,
    _Out_writes_to_(length, (patch_count + 1) * (patch_count + 1)) Vector3f* vertices,
    size_t length)
{
    assert(control_points.size() == (quadratic_bezier_control_point_count * quadratic_bezier_control_point_count));
    const auto curve_vertex_count = patch_count + 1;

    assert(curve_vertex_count * curve_vertex_count <= length);
    (void)length;

    // Generate all of the points.
    size_t ix = 0;
    for(unsigned int v = 0; v < curve_vertex_count; ++v)
    {
        // Range [0..1].
        const float t_v = v / static_cast<float>(patch_count);

        for(unsigned int u = 0; u < curve_vertex_count; ++u)
        {
            // Range [0..1].
            const float t_u = u / static_cast<float>(patch_count);

            vertices[ix] = calculate_quadratic_bezier_vertex(control_points, t_u, t_v);
            ix++;
        }
    }
}

// TODO: 2014: It would make much more sense to do this in a compute shader to generate the data where they are used.
std::vector<Vector3f> generate_patch_quadratic_bezier_vertex_array(
    const Control_point_patch& control_points,
    unsigned int patch_count)
{
    assert(control_points.size() == (quadratic_bezier_control_point_count * quadratic_bezier_control_point_count));
    const auto curve_vertex_count = patch_count + 1;

    std::vector<Vector3f> vertex_array;
    vertex_array.reserve(curve_vertex_count * curve_vertex_count);

    // Generate all of the points.
    for(unsigned int v = 0; v < curve_vertex_count; ++v)
    {
        // Range [0..1].
        const float t_v = v / static_cast<float>(patch_count);

        for(unsigned int u = 0; u < curve_vertex_count; ++u)
        {
            // Range [0..1].
            const float t_u = u / static_cast<float>(patch_count);

            vertex_array.emplace_back(calculate_quadratic_bezier_vertex(control_points, t_u, t_v));
        }
    }

    return vertex_array;
}

std::vector<uint16_t> generate_patch_index_array(unsigned int patch_count, size_t bias)
{
    const auto curve_vertex_count = patch_count + 1;

    // Verify that all indices generated will not wrap the max number of vertices.
    assert(bias + patch_count + patch_count * curve_vertex_count <= UINT16_MAX);

    std::vector<uint16_t> index_array;
    index_array.reserve(patch_count * patch_count);

    for(unsigned int vv = 0; vv < patch_count; ++vv)
    {
        for(unsigned int uu = 0; uu < patch_count; ++uu)
        {
            index_array.emplace_back(static_cast<uint16_t>(bias + (uu + 0) + (vv + 0) * curve_vertex_count));
            index_array.emplace_back(static_cast<uint16_t>(bias + (uu + 0) + (vv + 1) * curve_vertex_count));
            index_array.emplace_back(static_cast<uint16_t>(bias + (uu + 1) + (vv + 0) * curve_vertex_count));
            index_array.emplace_back(static_cast<uint16_t>(bias + (uu + 1) + (vv + 0) * curve_vertex_count));
            index_array.emplace_back(static_cast<uint16_t>(bias + (uu + 0) + (vv + 1) * curve_vertex_count));
            index_array.emplace_back(static_cast<uint16_t>(bias + (uu + 1) + (vv + 1) * curve_vertex_count));
        }
    }

    return index_array;
}

std::vector<Vector2f> generate_patch_texture_coords_array(unsigned int patch_count)
{
    const auto curve_vertex_count = patch_count + 1;

    std::vector<Vector2f> texture_coords_array;
    texture_coords_array.reserve(curve_vertex_count * curve_vertex_count);

    const float scale = 1.0f / patch_count;
    for(unsigned int vv = 0; vv < curve_vertex_count; ++vv)
    {
        for(unsigned int uu = 0; uu < curve_vertex_count; ++uu)
        {
            texture_coords_array.push_back({uu * scale, vv * scale});
        }
    }

    return texture_coords_array;
}

}

