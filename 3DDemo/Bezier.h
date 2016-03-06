#pragma once

namespace Demo
{

const unsigned int quadratic_bezier_control_point_count = 3;
struct Bezier_patch
{
    const struct Vector3f* control_point_vertices;
    unsigned int control_point_indices[quadratic_bezier_control_point_count * quadratic_bezier_control_point_count];
};

void append_quadratic_bezier_vertex_patch(const Bezier_patch& patch, unsigned int patch_count, std::vector<Vector3f>& vertices);

}

