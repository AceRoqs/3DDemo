#include "PreCompile.h"
#include "GenerateMesh.h"
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

namespace detail
{

Vector3f calculate_quadratic_bezier_vertex(const std::vector<struct Vector3f>& control_points, float t_u, float t_v)
{
    Vector3f vertex = {0.0f, 0.0f, 0.0f};

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

} // detail

}

