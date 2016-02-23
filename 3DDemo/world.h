#pragma once

#include <ImageProcessing/Bitmap.h>

namespace Demo
{

struct Vector3f;
struct Vector2f;

struct Polygon
{
    std::vector<unsigned int> vertex_indices;       // Indices into vertex list (used for load only).
    std::vector<unsigned int> texture_coordinates;  // Indices into texture coordinate list (used for load only).
    unsigned int texture;                           // Texture ID.
    unsigned int lightmap;                          // Index into lightmap list.

    Polygon();
};

struct Map
{
    std::vector<struct Demo::Polygon> world_mesh;
    //std::vector<struct Vector3f> vertices;
    //std::vector<struct Vector3f> vertices2;
    //unsigned int patch_count;
    unsigned int patch_texture_id;
    //class Emitter emitter;
};

std::istream& operator>>(std::istream& is, Demo::Polygon& polygon);

bool is_point_in_world(const struct Vector3f& point);

Map start_load(
    _In_z_ const char* file_name,
    std::vector<ImageProcessing::Bitmap>* texture_list,
    std::vector<Vector3f>* vertices,
    std::vector<Vector2f>* texture_coords);

}

