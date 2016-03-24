#pragma once

namespace Demo
{

const unsigned int quadratic_bezier_control_point_count = 3;
typedef std::vector<struct Vector3f> Control_point_patch;

void append_quadratic_bezier_vertex_patch(const Control_point_patch& control_points, unsigned int patch_count, std::vector<Vector3f>& vertices);
void generate_patch_quadratic_bezier_vertex_array(
    const Control_point_patch& control_points,
    unsigned int patch_count,
    _Out_writes_to_(length, (patch_count + 1) * (patch_count + 1)) Vector3f* vertices,
    size_t length);
std::vector<Vector3f> generate_patch_quadratic_bezier_vertex_array(
    const Control_point_patch& control_points,
    unsigned int patch_count);

}

