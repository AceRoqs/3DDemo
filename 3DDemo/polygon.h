#ifndef POLYGON_H
#define POLYGON_H

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

}

std::istream& operator>>(std::istream& is, Graphics::Polygon& polygon);

#endif

