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

static Vector3f calculate_quadratic_bezier_vertex(const Bezier_patch& patch, float t_u, float t_v)
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
            const size_t control_point_index = patch.control_point_indices[j * quadratic_bezier_control_point_count + i];
            vertex += patch.control_point_vertices[control_point_index] * basis;
        }
    }

    return vertex;
}

// TODO: 2014: It would make much more sense to do this in a compute shader to generate the data where they are used.
std::vector<Vector3f> generate_quadratic_bezier_vertex_patch(const Bezier_patch& patch, unsigned int patch_count)
{
    const auto curve_vertex_count = patch_count + 1;

    std::vector<Vector3f> vertices;
    vertices.reserve(curve_vertex_count * curve_vertex_count);

    // Generate all of the points.
    for(unsigned int v = 0; v < curve_vertex_count; ++v)
    {
        // Range [0..1].
        const float t_v = v / static_cast<float>(patch_count);

        for(unsigned int u = 0; u < curve_vertex_count; ++u)
        {
            // Range [0..1].
            const float t_u = u / static_cast<float>(patch_count);

            vertices.push_back(calculate_quadratic_bezier_vertex(patch, t_u, t_v));
        }
    }

    return vertices;
}

}

