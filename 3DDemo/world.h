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

struct Patch
{
    std::vector<Vector3f> vertices;
    std::vector<Vector2f> texture_coords;
    std::vector<uint16_t> index_array;
    unsigned int patch_count;
    unsigned int texture_id;
    size_t index_array_offset;
};

struct Map
{
    std::vector<struct Demo::Polygon> world_mesh;

    // TODO: This is a set of vertices for all of the polygons in the world_mesh.  The world shouldn't
    // be made of individual polygons in 2016, so this is maybe correct to put all world points in one vector
    // instead of individual arrays for each Polygon.  Just moving the vectors here to start to simplify the
    // renderer, but eventually the world_mesh idea needs to go away and replaced with something like
    // an octree of objects (including meshes).  Possibly for AZDO, there will still just be one big buffer,
    // but let's get all of the objects rendering with glDrawElements uniformly first.
    std::vector<Vector3f> vertices;
    std::vector<Vector2f> texture_coords;

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
    std::vector<ImageProcessing::Bitmap>* texture_list);

}

