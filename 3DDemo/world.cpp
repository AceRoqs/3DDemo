#include "PreCompile.h"
#include "world.h"
#include "GenerateMesh.h"
#include "BitmapReader.h"
#include "LinearAlgebra.h"
#include <PortableRuntime/CheckException.h>

namespace Demo
{

constexpr Particle_descriptor particle_descriptor =
{
    // Position.
    0.5f,       // x scale.
    -0.25f,     // x bias.
    0.0f,       // y scale.
    0.0f,       // y bias.
    0.0f,       // z scale.
    0.0f,       // z bias.

    // Velocity.
    -0.004f,    // x scale.
    0.0016f,    // x bias.
    0.008f,     // y scale.
    0.0f,       // y bias.
    -0.004f,    // z scale.
    0.0016f,    // z bias.

    150.0f,     // life.
};

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

static std::vector<Vector3f> read_vertex_array(std::istream& is, unsigned int num_points)
{
    std::vector<Vector3f> vertex_array;
    for(unsigned int ix = 0; ix < num_points; ++ix)
    {
        Vector3f vertex;
        is >> vertex;
        vertex_array.emplace_back(vertex);
    }

    return vertex_array;
}

static std::vector<Vector2f> read_scaled_quad_texture_coords_array(std::istream& is)
{
    float scale_x, scale_y;
    is >> scale_x >> scale_y;

    return std::vector<Vector2f>{{0.0f,    0.0f},
                                 {scale_x, 0.0f},
                                 {0.0f,    scale_y},
                                 {scale_x, scale_y}};
}

static std::vector<uint16_t> generate_biased_quad_index_array(unsigned int bias)
{
    return {static_cast<uint16_t>(bias + 0),
            static_cast<uint16_t>(bias + 2),
            static_cast<uint16_t>(bias + 1),
            static_cast<uint16_t>(bias + 1),
            static_cast<uint16_t>(bias + 2),
            static_cast<uint16_t>(bias + 3)};
}

static std::tuple<std::vector<Polygon>, std::vector<Vector3f>, std::vector<Vector2f>> read_world_mesh(std::istream& is)
{
    unsigned int polygon_count;
    is >> polygon_count;

    // TODO: 2016: Get firm on types put in index buffers (int16_t vs uint16_t), and what the range is and why.
    constexpr unsigned int quad_point_count = 4;
    assert(polygon_count < 65536 / quad_point_count);

    std::vector<Polygon> world_mesh;
    world_mesh.reserve(polygon_count);

    std::vector<Vector3f> vertex_array;
    vertex_array.reserve(polygon_count * quad_point_count);

    std::vector<Vector2f> texture_coords_array;
    texture_coords_array.reserve(polygon_count * 4);

    for(unsigned int ii = 0; ii < polygon_count; ++ii)
    {
        // TODO: 2016: If vertex arrays are shared across polygons, then indexes should just be an index into world_vertices.
        std::vector<Vector3f> vertex_subarray{read_vertex_array(is, quad_point_count)};
        std::vector<Vector2f> texture_coords_subarray{read_scaled_quad_texture_coords_array(is)};
        std::vector<uint16_t> index_subarray{generate_biased_quad_index_array(ii * quad_point_count)};

        unsigned int texture_id, lightmap_id;
        is >> texture_id >> lightmap_id;

        // TODO: 2016: Consider changing read* implementations to take a back_insert_iterator directly.
        std::move(std::begin(vertex_subarray), std::end(vertex_subarray), std::back_inserter(vertex_array));
        std::move(std::begin(texture_coords_subarray), std::end(texture_coords_subarray), std::back_inserter(texture_coords_array));
        world_mesh.push_back({std::move(index_subarray), texture_id, lightmap_id});
    }

    return std::make_tuple(world_mesh, vertex_array, texture_coords_array);
}

static std::vector<Implicit_surface> read_implicit_surfaces(std::istream& is)
{
    unsigned int implicit_surface_count;
    is >> implicit_surface_count;

    std::vector<Implicit_surface> implicit_surfaces;
    implicit_surfaces.reserve(implicit_surface_count);

    Control_point_patch bezier_control_points;
    bezier_control_points.reserve(quadratic_bezier_control_point_count * quadratic_bezier_control_point_count);
    for(unsigned int ii = 0; ii < implicit_surface_count; ++ii)
    {
        unsigned int implicit_surface_texture_id;
        is >> implicit_surface_texture_id;

        Vector3f implicit_surface_origin;
        is >> implicit_surface_origin;

        // The move at the end of the loop guarantees this.
        assert(bezier_control_points.size() == 0);

        for(unsigned int jj = 0; jj < quadratic_bezier_control_point_count * quadratic_bezier_control_point_count; ++jj)
        {
            Vector3f control_point;
            is >> control_point;
            bezier_control_points.emplace_back(control_point);
        }

        implicit_surfaces.push_back({std::move(bezier_control_points), implicit_surface_texture_id, implicit_surface_origin});
    }

    return implicit_surfaces;
}

static std::vector<Emitter> read_emitters(std::istream& is)
{
    unsigned int emitter_count;
    is >> emitter_count;

    std::vector<Emitter> emitters;
    emitters.reserve(emitter_count);

    for(unsigned int ii = 0; ii < emitter_count; ++ii)
    {
        Vector3f origin;
        is >> origin;

        unsigned int particle_count;
        is >> particle_count;

        unsigned int texture_id;
        is >> texture_id;

        emitters.push_back({origin, particle_count, particle_descriptor, texture_id});
    }

    return emitters;
}

static Map load_world_data(
    std::istream& is,
    std::vector<ImageProcessing::Bitmap>* texture_list)
{
    unsigned int cTextures;
    is >> cTextures;

    for(unsigned int ii = 0; ii < cTextures; ++ii)
    {
        // TODO: 2016: Nice buffer overrun here.  This is trusted data, but it should still read into a string.
        char file_name[MAX_PATH];
        is >> file_name;
        texture_list->emplace_back(bitmap_from_file(file_name));
    }

    std::vector<Polygon> world_mesh;
    std::vector<Vector3f> vertex_array;
    std::vector<Vector2f> texture_coords_array;

    std::tie(world_mesh, vertex_array, texture_coords_array) = read_world_mesh(is);

    std::vector<Implicit_surface> implicit_surfaces = read_implicit_surfaces(is);
    std::vector<Emitter> emitters = read_emitters(is);

    return {std::move(world_mesh),
            std::move(implicit_surfaces),
            std::move(emitters),
            std::move(vertex_array),
            std::move(texture_coords_array)};
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

