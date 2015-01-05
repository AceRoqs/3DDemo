#pragma once

#include "Bitmap.h"

namespace Demo
{

struct Polygon;
struct Vector3f;
struct Vector2f;
struct Map;
class Emitter;
struct Camera;

void initialize_gl_constants();
void initialize_gl_world_data(const std::vector<ImageProcessing::Bitmap>& texture_list, const std::vector<Vector3f>& vertices, const std::vector<Vector2f>& texture_coords);
void draw_list(const Map& map, const std::vector<Vector3f>& vertices, const std::vector<Vector3f>& vertices2, unsigned int patch_count, unsigned int patch_texture_id, const Emitter& emitter, const Camera& camera);

}

