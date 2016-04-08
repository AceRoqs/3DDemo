#pragma once

namespace Demo
{

const unsigned int quadratic_bezier_control_point_count = 3;
typedef std::vector<struct Vector3f> Control_point_patch;

std::vector<struct Vector3f> generate_quadratic_bezier_vertex_array(const Control_point_patch& control_points, unsigned int patch_count);
std::vector<uint16_t> generate_implicit_surface_index_array(unsigned int patch_count, size_t bias);
std::vector<struct Vector2f> generate_implicit_surface_texture_coords_array(unsigned int patch_count);

}

