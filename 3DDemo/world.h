#ifndef WORLD_H
#define WORLD_H

namespace Graphics
{

struct Polygon
{
    std::vector<unsigned int> vertex_indices;       // Indices into vertex list (used for load only).
    std::vector<unsigned int> texture_coordinates;  // Indices into texture coordinate list (used for load only).
    unsigned int texture;                           // Texture ID.
    unsigned int lightmap;                          // Index into lightmap list.

    Polygon();
};

} // namespace

struct Map
{
    std::vector<struct Graphics::Polygon> world_mesh;
    //std::vector<struct Vector3f> vertices;
    //std::vector<struct Vector3f> vertices2;
    //unsigned int patch_count;
    //unsigned int patch_texture_id;
    //class Emitter emitter;
};

std::istream& operator>>(std::istream& is, Graphics::Polygon& polygon);

bool is_point_in_world(const struct Vector3f& point);

Map start_load(
    _In_z_ const char* file_name,
    std::vector<struct Bitmap>* texture_list,
    std::vector<struct Vector3f>* vertices,
    std::vector<struct Vector2f>* texture_coords);

#endif

