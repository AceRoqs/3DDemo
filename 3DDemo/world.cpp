//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: View information for world
//=========================================================================
#include "PreCompile.h"
#include "world.h"
#include "Bitmap.h"
#include "LinearAlgebra.h"

// TODO: 2014: This should not be extern.
const float g_WorldVector[] =
{
    -2, -2, -10,   // left lower front      0
    -2,  2, -10,   // left upper front      1
     2, -2, -10,   // right lower front     2
     2,  2, -10,   // right upper front     3
     2, -2,   0,   // right lower back      4
    -2, -2,   0,   // left lower back       5
    -2,  2,   0,   // left upper back       6
     2,  2,   0,   // right upper back      7
    // sky
    -500,  50, -500,  // left upper front   8
    -500,  50, 500,   // left upper back    9
     500,  50, 500,   // right upper back   10
     500,  50, -500,  // right upper front  11

    // outside wall
    -10, -2, -20,   // left lower front     12
    -10,  2, -20,   // left upper front     13
     10, -2, -20,   // right lower front    14
     10,  2, -20,   // right upper front    15
    // far floor
     10, -2, -20,   //                      16
     10, -2, -10,   //                      17
    -10, -2, -10,   //                      18
    // left far wall
    -10, 2, -10,        //                      19

     10, 2, -10,        //                      20
     10, 4, -10,        //                      21
    -10, 4, -10,        //                      22
      2, 4, -10,        //                      23
     -2, 4, -10,        //                      24
     10,-2, -15,        //                      25
     10, 2, -15,        //                      26
     10,-2, -16,        //                      27
     10, 2, -16,        //                      28
    // bezier curve definitions
    -2, 0, -10,     // 0 29
    -2, 0, -11,     // 1 30
    -3, 0, -11,     // 2 31
    -2,-1, -10,     // 3 32
    -2,-1, -11,     // 4 33
    -3,-1, -11,     // 5 34
    //-2,-2, -10,       // 6 dupe 0
    -2,-2, -11,     // 7 35
    -3,-2, -11,     // 8 36
    -4, 0, -11,     // 9 37
    -4, 0, -10,     // 10 38
    -4,-1, -11,     // 11 39
    -4,-1, -10,     // 12 40
    -4,-2, -11,     // 13 41
    -4,-2, -10      // 14 42
};

static const float WorldTexture[] =
{
    0.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    1.0, 1.0,
    8.0, 0.0,
    8.0, 1.0,
    5.0, 0.0,
    5.0, 1.0,
    2.5, 0.0,
    2.5, 1.0,
};

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

// Returns true if the point is inside the bounds of all polygons in the world.
bool is_point_in_world(float x, float y, float z)
{
    (y);    // unreferenced parameter

//    return true;
    // TODO: finishme
    if(x < -9.0 || x > 9.0)
    {
        return false;
    }
    if(z < 1.0 || z > 19.0)
    {
        return false;
    }
    if(z < 11.0)
    {
        if(x < -1.25 || x > 1.25)
        {
            return false;
        }
    }
    return true;
}

static void load_world_data(
    std::istream& is,
    std::vector<Bitmap>* texture_list,
    std::vector<Graphics::Polygon>* polys,
    std::vector<Vector3f>* vertex_formats,
    std::vector<Vector2f>* texture_coords)
{
    unsigned int cTextures;

    is >> cTextures;

    unsigned int ii;
    for(ii = 0; ii < cTextures; ++ii)
    {
        char file_name[MAX_PATH];
        is >> file_name;
        texture_list->push_back(bitmap_from_file(file_name));
    }

    unsigned int cPolys;
    is >> cPolys;

    for(ii = 0; ii < cPolys; ++ii)
    {
        Graphics::Polygon poly;
        is >> poly;
        polys->push_back(poly);

        for(auto jj = 0; jj < 4; ++jj)
        {
            auto ix = poly.points[jj];
            Vector3f out_vertex_format;
            out_vertex_format.element[0] = g_WorldVector[ix * 3];
            out_vertex_format.element[1] = g_WorldVector[ix * 3 + 1];
            out_vertex_format.element[2] = g_WorldVector[ix * 3 + 2];
            vertex_formats->push_back(out_vertex_format);

            ix = poly.texture_coordinates[jj];
            Vector2f out_coord;
            out_coord.element[0] = WorldTexture[ix * 2];
            out_coord.element[1] = WorldTexture[ix * 2 + 1];
            texture_coords->push_back(out_coord);
        }
    }
}

void start_load(
    _In_ char* file_name,
    std::vector<Bitmap>* texture_list,
    std::vector<Graphics::Polygon>* polys,
    std::vector<Vector3f>* vertex_formats,
    std::vector<Vector2f>* texture_coords)
{
    std::ifstream fis;
    fis.open(file_name);
    if(fis.is_open())
    {
        load_world_data(fis, texture_list, polys, vertex_formats, texture_coords);
        fis.close();
    }
}

