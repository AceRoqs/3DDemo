#include "PreCompile.h"
#include "render.h"
#include "particle.h"
#include "world.h"
#include "Bezier.h"
#include <ImageProcessing/Bitmap.h>
#include "Camera.h"
#include "LinearAlgebra.h"

namespace Demo
{

// Vertices is a two dimensional array of patch vertices.  generate_quadratic_bezier_quads() creates the expected output.
// TODO: 2016: generate_quadratic_bezier_quads doesn't generate quads.  It generates a curve_vertex_count x curve_vertex_count array of vertices.
// TODO: 2016: Pass in a Patch object, with verts, textures (id and coords), and patch_count.
static void draw_patch(const Camera& camera, const Patch& patch, unsigned int patch_count, unsigned int texture_id)
{
    const float scale = 1.0f / patch_count;
    const auto curve_vertex_count = patch_count + 1;

    // TODO: 2016: index_array and texture_coords (and vertices) can be cached, as long as patch_count doesn't change between frames.
    // TODO: 2016: Calculate this stuff at the same time the vertices are calculated.
    // TODO: 2016: When reusing arrays, can precalculate the reserve to be the largest expected size, so reallocation never happens.
    std::vector<uint16_t> index_array;
    index_array.reserve(patch_count * patch_count * 6);     // Six push_backs per loop iteration.

    std::vector<Vector2f> texture_coords;
    texture_coords.reserve(curve_vertex_count * curve_vertex_count);

    for(unsigned int vv = 0; vv < patch_count; ++vv)
    {
        for(unsigned int uu = 0; uu < patch_count; ++uu)
        {
            index_array.push_back(static_cast<uint16_t>((uu + 0) + (vv + 0) * curve_vertex_count));
            index_array.push_back(static_cast<uint16_t>((uu + 0) + (vv + 1) * curve_vertex_count));
            index_array.push_back(static_cast<uint16_t>((uu + 1) + (vv + 0) * curve_vertex_count));
            index_array.push_back(static_cast<uint16_t>((uu + 1) + (vv + 0) * curve_vertex_count));
            index_array.push_back(static_cast<uint16_t>((uu + 0) + (vv + 1) * curve_vertex_count));
            index_array.push_back(static_cast<uint16_t>((uu + 1) + (vv + 1) * curve_vertex_count));
        }
    }

    for(unsigned int vv = 0; vv < curve_vertex_count; ++vv)
    {
        for(unsigned int uu = 0; uu < curve_vertex_count; ++uu)
        {
            texture_coords.push_back({uu * scale, vv * scale});
        }
    }

    // GL_MODELVIEW assumed.
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_ONE, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexPointer(3, GL_FLOAT, 0, &patch.vertices[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &texture_coords[0]);

    assert(index_array.size() < INT_MAX);   // GLsizei == int
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_array.size()), GL_UNSIGNED_SHORT, &index_array[0]);
}

// TODO: 2016: This isn't a true billboard, in the sense that it's only billboarded against the vertical axis.
// TODO: 2016: Pass in a Billboard object, with center_position, vertices, texture (ID and coords).
// TODO: 2016: Precalculate a full vertex array for the Emitter.  Texture and index array will be static.  Then,
// compare the draw_billboard function with draw_patch, and see if there are any material differences left.
static void draw_billboard(const Camera& camera, const Vector3f& position, float size, unsigned int texture_id)
{
    // Vertices are specified counterclockwise from the upper-left corner.
    // TODO: 2016: Specify vertices in a flat array, like patch.
    // TODO: 2016: Eliminate half_size and just premultiply size by 0.5f.
    const float half_size = size / 2.0f;
    const Vector3f vertices[] = {{ -half_size,  half_size, 0.0f },
                                 { -half_size, -half_size, 0.0f },
                                 {  half_size, -half_size, 0.0f },
                                 {  half_size,  half_size, 0.0f }};

    const Vector2f texture_coords[] = {{ 0.0f, 0.0f },
                                       { 0.0f, 1.0f },
                                       { 1.0f, 1.0f },
                                       { 1.0f, 0.0f }};

    const uint8_t index_array[] = { 0, 1, 3, 3, 1, 2 };

    // Transform to location.
    // GL_MODELVIEW assumed.
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());
    glTranslatef(position.x(), position.y(), position.z());

    // Billboard the sprite.
    glRotatef(-camera.m_degrees, 0.0f, 1.0f, 0.0f);

    // TODO: 2016: Understand why glColor4f only passes red.
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &texture_coords[0]);

    glDrawElements(GL_TRIANGLES, ARRAYSIZE(index_array), GL_UNSIGNED_BYTE, index_array);
}

// TODO: 2014: Drawing should be done against a vertex/index array.
static void draw_emitter(const Emitter& emitter, const Camera& camera, unsigned int texture_id)
{
    std::for_each(emitter.cbegin(), emitter.cend(), [&, texture_id](const Particle& particle)
    {
        // TODO: 2016: size should be a member of Emitter.
        draw_billboard(camera, particle.position, 0.5f, texture_id);
    });
}

// TODO: 2016: Move non-draw functions to the top of this module.
static void bind_bitmap_to_gl_texture(const ImageProcessing::Bitmap& bitmap, unsigned int texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // TODO: GL_REPEAT seems like a good idea, but something about GL_LINEAR is causing
    // v=0 to actually wrap the texture at least vertically one line.  Likely this is
    // a bug in the renderer itself, as it happens in more than one OpenGL implementation.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if(bitmap.filtered)
    {
        // Bilinear filtering.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        // Don't filter the texture.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 3,
                 bitmap.xsize,
                 bitmap.ysize,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 &bitmap.bitmap[0]);
}

void initialize_gl_constants()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const double LEFTCLIP = -0.5;
    const double RIGHTCLIP = 0.5;
    const double BOTTOMCLIP = -0.5;
    const double TOPCLIP = 0.5;
    const double NEARCLIP = 0.5;
    const double FARCLIP = 700.0;

    // TODO: 2014: Use these bounds once the world geometry collision has knowledge of the clip planes.
    //const double LEFTCLIP = -1.0;
    //const double RIGHTCLIP = 1.0;
    //const double BOTTOMCLIP = -1.0;
    //const double TOPCLIP = 1.0;
    //const double NEARCLIP = 1.0;
    //const double FARCLIP = 512.0;
    glFrustum(LEFTCLIP, RIGHTCLIP, BOTTOMCLIP, TOPCLIP, NEARCLIP, FARCLIP);

    glMatrixMode(GL_MODELVIEW);

    // Enable backface culling and hidden surface removal.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);

    // Enable vertex arrays.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void initialize_gl_world_data(
    const std::vector<ImageProcessing::Bitmap>& texture_list)
{
    // Load all texture data.
    assert(texture_list.size() <= UINT_MAX);
    const auto size = static_cast<unsigned int>(texture_list.size());
    for(unsigned int ix = 0; ix < size; ++ix)
    {
        bind_bitmap_to_gl_texture(texture_list[ix], ix);
    }
}

// TODO: modularize into separate functions
void draw_map(
    const Map& map,
    const struct Camera& camera,
    const Patch& patch1,
    const Patch& patch2,
    unsigned int patch_count,
    const class Emitter& emitter)
{
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);

    // GL_MODELVIEW assumed.
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());

    glVertexPointer(3, GL_FLOAT, 0, &map.vertices[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &map.texture_coords[0]);

    // single loop multi pass textured lighting
    // this is done is one pass because of visibility
    // problems on a second light pass once the world is drawn
    for(unsigned int ii = 0; ii < map.world_mesh.size(); ++ii)
    {
        // TODO: 2016: Convert all index arrays to 16-bit.
        assert(map.world_mesh.size() < 256 / 4);

        const uint8_t index_array[] =
        {
            static_cast<uint8_t>(ii * 4),
            static_cast<uint8_t>(ii * 4 + 1),
            static_cast<uint8_t>(ii * 4 + 2),
            static_cast<uint8_t>(ii * 4 + 3)
        };

        const Polygon* iter = &map.world_mesh[ii];

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDepthFunc(GL_LESS);
        glBlendFunc(GL_ONE, GL_ZERO);
        glBindTexture(GL_TEXTURE_2D, iter->texture);

        // TODO: 2016: Make this GL_TRIANGLES.  This might require changes to the persisted formats?
        glDrawElements(GL_QUADS, ARRAYSIZE(index_array), GL_UNSIGNED_BYTE, index_array);

        if(iter->lightmap == 0)
        {
            continue;
        }

        glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
        glDepthFunc(GL_EQUAL);
        glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, iter->lightmap);

        glDrawElements(GL_QUADS, ARRAYSIZE(index_array), GL_UNSIGNED_BYTE, index_array);
    }

    draw_patch(camera, patch1, patch_count, map.patch_texture_id);
    draw_patch(camera, patch2, patch_count, map.patch_texture_id);

    draw_emitter(emitter, camera, 6);

    assert(glGetError() == GL_NO_ERROR);
}

}

