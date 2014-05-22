//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: View information for world
//=========================================================================
#include "PreCompile.h"
#include "world.h"
#include "Bitmap.h"
#include "LinearAlgebra.h"

static const Vector3f world_vertices[] =
{
    { -2, -2, -10 },        // left lower front      0
    { -2,  2, -10 },        // left upper front      1
    {  2, -2, -10 },        // right lower front     2
    {  2,  2, -10 },        // right upper front     3
    {  2, -2,   0 },        // right lower back      4
    { -2, -2,   0 },        // left lower back       5
    { -2,  2,   0 },        // left upper back       6
    {  2,  2,   0 },        // right upper back      7
    // sky
    { -500, 50, -500 },     // left upper front      8
    { -500, 50,  500 },     // left upper back       9
    {  500, 50,  500 },     // right upper back     10
    {  500, 50, -500 },     // right upper front    11

    // outside wall
    { -10, -2, -20 },       // left lower front     12
    { -10,  2, -20 },       // left upper front     13
    {  10, -2, -20 },       // right lower front    14
    {  10,  2, -20 },       // right upper front    15
    // far floor
    {  10, -2, -20 },       //                      16
    {  10, -2, -10 },       //                      17
    { -10, -2, -10 },       //                      18
    // left far wall
    { -10, 2, -10 },        //                      19

    {  10, 2, -10 },        //                      20
    {  10, 4, -10 },        //                      21
    { -10, 4, -10 },        //                      22
    {   2, 4, -10 },        //                      23
    {  -2, 4, -10 },        //                      24
};

static const Vector2f world_texture_coords[] =
{
    { 0.0, 0.0 },
    { 0.0, 1.0 },
    { 1.0, 0.0 },
    { 1.0, 1.0 },
    { 8.0, 0.0 },
    { 8.0, 1.0 },
    { 5.0, 0.0 },
    { 5.0, 1.0 },
    { 2.5, 0.0 },
    { 2.5, 1.0 },
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
// TODO: Take Vector3f.
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
    std::vector<Vector3f>* vertices,
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
            // TODO: 2014: Bounds check constant arrays.
            auto ix = poly.points[jj];
            vertices->push_back(world_vertices[ix]);

            ix = poly.texture_coordinates[jj];
            texture_coords->push_back(world_texture_coords[ix]);
        }
    }
}

void start_load(
    _In_ char* file_name,
    std::vector<Bitmap>* texture_list,
    std::vector<Graphics::Polygon>* polys,
    std::vector<Vector3f>* vertices,
    std::vector<Vector2f>* texture_coords)
{
    std::ifstream fis;
    fis.open(file_name);
    if(fis.is_open())
    {
        load_world_data(fis, texture_list, polys, vertices, texture_coords);
        fis.close();
    }
}

