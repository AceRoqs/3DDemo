#ifndef WORLD_H
#define WORLD_H

namespace Graphics
{
    struct Polygon;
}

struct Position_vertex
{
    GLfloat aVertex[3];
};

struct TexCoord
{
    GLfloat aTexCoord[2];
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

