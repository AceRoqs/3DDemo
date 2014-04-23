#ifndef WORLD_H
#define WORLD_H

bool is_point_in_world(float x, float y, float z);

void start_load(
    _In_ char* szFileName,
    std::vector<class CPolygon>* polys,
    std::vector<struct Position_vertex>* vertex_formats,
    std::vector<struct TexCoord>* texture_coords);

extern const float g_WorldVector[];

#endif

