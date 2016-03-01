#pragma once

#include <ImageProcessing/Bitmap.h>

namespace Demo
{

struct Polygon;
struct Vector3f;
struct Vector2f;
struct Map;
struct Patch;
class Emitter;
struct Camera;

void initialize_gl_constants();
void initialize_gl_world_data(const std::vector<ImageProcessing::Bitmap>& texture_list);
void draw_map(const Map& map, const Camera& camera, const Patch& patch1, const Patch& patch2, const Emitter& emitter);

}

