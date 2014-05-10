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

struct Position_vertex
{
    float aVertex[3];
};

struct TexCoord
{
    float aTexCoord[2];
};

bool is_point_in_world(float x, float y, float z);

void start_load(
    _In_ char* szFileName,
    std::vector<std::string>* texture_list,
    std::vector<struct Graphics::Polygon>* polys,
    std::vector<struct Position_vertex>* vertex_formats,
    std::vector<struct TexCoord>* texture_coords);

extern const float g_WorldVector[];

#endif

