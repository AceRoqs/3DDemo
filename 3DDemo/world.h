#ifndef WORLD_H
#define WORLD_H

namespace Graphics
{

struct Polygon
{
    std::vector<int> points;                // Indices into vertex list (used for load only).
    std::vector<int> texture_coordinates;   // Indices into texture coordinate list (used for load only).
    unsigned int texture;                   // Texture ID.
    unsigned int lightmap;                  // Index into lightmap list.

    Polygon();
};

} // namespace

std::istream& operator>>(std::istream& is, Graphics::Polygon& polygon);

bool is_point_in_world(float x, float y, float z);

void start_load(
    _In_ char* file_name,
    std::vector<struct Bitmap>* texture_list,
    std::vector<struct Graphics::Polygon>* polys,
    std::vector<struct Vector3f>* vertex_formats,
    std::vector<struct Vector2f>* texture_coords);

extern const float g_WorldVector[];

#endif

