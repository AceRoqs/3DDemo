#pragma once

namespace Demo
{

const unsigned int quadratic_bezier_control_point_count = 3;
struct Bezier_patch
{
    const struct Vector3f* control_point_vertices;
    unsigned int control_point_indices[quadratic_bezier_control_point_count * quadratic_bezier_control_point_count];
};

std::vector<struct Vector3f> generate_quadratic_bezier_quads(const Bezier_patch& patch, unsigned int patch_count);

}

