#include "PreCompile.h"
#include "world.h"
#include "Bezier.h"
#include "BitmapReader.h"
#include "LinearAlgebra.h"
#include <PortableRuntime/CheckException.h>

namespace Demo
{

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

// TODO: 2016: Bezier patch data should come from file.
static const Vector3f bezier_control_points[] =
{
    { -2.0f, 0.0f, -10.0f },    // 0
    { -2.0f, 0.0f, -11.0f },    // 1
    { -3.0f, 0.0f, -11.0f },    // 2
    { -2.0f,-1.0f, -10.0f },    // 3
    { -2.0f,-1.0f, -11.0f },    // 4
    { -3.0f,-1.0f, -11.0f },    // 5
    { -2.0f,-2.0f, -10.0f },    // 6
    { -2.0f,-2.0f, -11.0f },    // 7
    { -3.0f,-2.0f, -11.0f },    // 8
};

static const Vector3f bezier_control_points2[] =
{
    { -3.0f, 0.0f, -11.0f },    // 0
    { -4.0f, 0.0f, -11.0f },    // 1
    { -4.0f, 0.0f, -10.0f },    // 2
    { -3.0f,-1.0f, -11.0f },    // 3
    { -4.0f,-1.0f, -11.0f },    // 4
    { -4.0f,-1.0f, -10.0f },    // 5
    { -3.0f,-2.0f, -11.0f },    // 6
    { -4.0f,-2.0f, -11.0f },    // 7
    { -4.0f,-2.0f, -10.0f },    // 8
};

static const Bezier_patch patches[] =
{
    { { bezier_control_points } },
    { { bezier_control_points2 } },
};

Polygon::Polygon() :
    texture(0),
    lightmap(0)
{
}

std::istream& operator>>(std::istream& is, Demo::Polygon& polygon)
{
    // Clear and realloc vectors.
    std::vector<unsigned int>().swap(polygon.vertex_indices);
    std::vector<unsigned int>().swap(polygon.texture_coordinates);

    unsigned int num_points;
    is >> num_points;
    if(num_points > 0)
    {
        polygon.vertex_indices.reserve(num_points);
        polygon.texture_coordinates.reserve(num_points);

        for(unsigned int ix = 0; ix < num_points; ++ix)
        {
            unsigned int vertex_indices;
            is >> vertex_indices;
            polygon.vertex_indices.push_back(vertex_indices);
        }

        for(unsigned int ix = 0; ix < num_points; ++ix)
        {
            unsigned int texture_coordinate;
            is >> texture_coordinate;
            polygon.texture_coordinates.push_back(texture_coordinate);
        }
    }

    is >> polygon.texture >> polygon.lightmap;
    return is;
}

// Returns true if the point is inside the bounds of all polygons in the world.
bool is_point_in_world(const Vector3f& point)
{
    // TODO: Use the world geometry to determine this.
    if(point.x() < -9.0 || point.x() > 9.0)
    {
        return false;
    }
    if(point.z() < 1.0 || point.z() > 19.0)
    {
        return false;
    }
    if(point.z() < 11.0)
    {
        if(point.x() < -1.25 || point.x() > 1.25)
        {
            return false;
        }
    }
    return true;
}

static Map load_world_data(
    std::istream& is,
    std::vector<ImageProcessing::Bitmap>* texture_list)
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

    unsigned int patch_texture_id;
    is >> patch_texture_id;

    unsigned int cPolys;
    is >> cPolys;

    Map map;
    map.patch_texture_id = patch_texture_id;

    for(ii = 0; ii < cPolys; ++ii)
    {
        Demo::Polygon poly;
        is >> poly;
        map.world_mesh.push_back(poly);

        for(auto jj = 0; jj < 4; ++jj)
        {
            // TODO: 2014: Bounds check constant arrays.
            auto ix = poly.vertex_indices[jj];
            CHECK_EXCEPTION(ix < ARRAYSIZE(world_vertices), u8"world_vertices too small for index");
            map.vertices.push_back(world_vertices[ix]);

            ix = poly.texture_coordinates[jj];
            CHECK_EXCEPTION(ix < ARRAYSIZE(world_texture_coords), u8"world_texture_coords too small for index");
            map.texture_coords.push_back(world_texture_coords[ix]);
        }
    }

    map.patches.reserve(2);
    map.patches.push_back(patches[0]);
    map.patches.push_back(patches[1]);

    return map;
}

// TODO: Ensure file_name is UTF-8.
Map start_load(
    _In_z_ const char* file_name,
    std::vector<ImageProcessing::Bitmap>* texture_list)
{
    std::ifstream fis;
    fis.open(file_name);
    CHECK_EXCEPTION(fis.is_open(), std::string("Could not open file:") + file_name);

    return load_world_data(fis, texture_list);
}

}

