#pragma once

#include <ImageProcessing/Bitmap.h>

namespace Demo
{

struct Map;
struct Dynamic_meshes;
class Emitter;
struct Camera;

class Renderer
{
public:
    Renderer();
    static std::vector<unsigned int> initialize_gl_world_data(const std::vector<ImageProcessing::Bitmap>& texture_list);
    static void deinitialize_gl_world_data(const std::vector<unsigned int>& texture_ids);
    static void draw_map(const Map& map, const std::vector<unsigned int>& texture_ids, const Dynamic_meshes& dynamic_meshes, const Camera& camera);
};


}

