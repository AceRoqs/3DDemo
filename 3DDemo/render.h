#pragma once

#include <ImageProcessing/Bitmap.h>

namespace Demo
{

struct Map;
struct Dynamic_meshes;
class Emitter;
struct Camera;

void initialize_gl_constants();
void initialize_gl_world_data(const std::vector<ImageProcessing::Bitmap>& texture_list);
void draw_map(const Map& map, const Dynamic_meshes& dynamic_meshes, const Camera& camera);

}

