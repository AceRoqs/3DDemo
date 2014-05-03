#include "PreCompile.h"
#include "polygon.h"

namespace Graphics
{

Polygon::Polygon() :
    texture(0),
    lightmap(0)
{
}

}

std::istream& operator>>(std::istream& is, Graphics::Polygon& polygon)
{
    // Clear and realloc vectors.
    std::vector<int>().swap(polygon.points);
    std::vector<int>().swap(polygon.texture_coordinates);

    unsigned int num_points;
    is >> num_points;
    if(num_points > 0)
    {
        polygon.points.reserve(num_points);
        polygon.texture_coordinates.reserve(num_points);

        for(unsigned int ix = 0; ix < num_points; ++ix)
        {
            int point;
            is >> point;
            polygon.points.push_back(point);
        }

        for(unsigned int ix = 0; ix < num_points; ++ix)
        {
            int texture_coordinate;
            is >> texture_coordinate;
            polygon.texture_coordinates.push_back(texture_coordinate);
        }
    }

    is >> polygon.texture >> polygon.lightmap;
    return is;
}

