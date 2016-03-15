#pragma once

namespace Demo
{

const unsigned int quadratic_bezier_control_point_count = 3;
struct Bezier_patch
{
    const struct Vector3f* control_point_vertices;
};

void append_quadratic_bezier_vertex_patch(const Bezier_patch& patch, unsigned int patch_count, std::vector<Vector3f>& vertices);
void generate_patch_quadratic_bezier_vertex_array(
    const Bezier_patch& patch,
    unsigned int patch_count,
    _Out_writes_to_(length, (patch_count + 1) * (patch_count + 1)) Vector3f* vertices,
    size_t length);

}

