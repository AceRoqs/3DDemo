#ifndef BEZIER_H
#define BEZIER_H

const unsigned int quadratic_bezier_control_point_count = 3;
struct bezier_patch
{
    const struct Vector3f* control_point_vertices;
    unsigned int control_point_indices[quadratic_bezier_control_point_count * quadratic_bezier_control_point_count];
};

std::vector<struct Vector3f> generate_quadratic_bezier_quads(const bezier_patch& patch, unsigned int patch_count);

#endif

