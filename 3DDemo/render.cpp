#include "PreCompile.h"
#include "render.h"
#include "particle.h"
#include "world.h"
#include <ImageProcessing/Bitmap.h>
#include "Camera.h"
#include "LinearAlgebra.h"

namespace Demo
{

static void bind_bitmap_to_gl_texture(const ImageProcessing::Bitmap& bitmap, unsigned int texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // TODO: 2016: GL_REPEAT seems like a good idea, but something about GL_LINEAR is causing
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

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 bitmap.xsize,
                 bitmap.ysize,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 bitmap.bitmap.data());
}

// TODO: 2016: To optimize setting this state when it is already set, it might be useful to have
// an enumeration on the type of projection set - default perspective, orthographic, custom, etc.
static void initialize_default_projection_matrix()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    constexpr double LEFTCLIP{-0.5};
    constexpr double RIGHTCLIP{0.5};
    constexpr double BOTTOMCLIP{-0.5};
    constexpr double TOPCLIP{0.5};
    constexpr double NEARCLIP{0.5};
    constexpr double FARCLIP{700.0};

    // TODO: 2014: Use these bounds once the world geometry collision has knowledge of the clip planes.
    //constexpr double LEFTCLIP{-1.0};
    //constexpr double RIGHTCLIP{1.0};
    //constexpr double BOTTOMCLIP{-1.0};
    //constexpr double TOPCLIP{1.0};
    //constexpr double NEARCLIP{1.0};
    //constexpr double FARCLIP{512.0};
    glFrustum(LEFTCLIP, RIGHTCLIP, BOTTOMCLIP, TOPCLIP, NEARCLIP, FARCLIP);
}

void initialize_gl_constants()
{
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

static void draw_dynamic_meshes(const std::vector<Implicit_surface>& implicit_surfaces, const Dynamic_meshes& dynamic_meshes, const Camera& camera)
{
    assert(implicit_surfaces.size() == dynamic_meshes.implicit_surfaces.size());

    initialize_default_projection_matrix();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_ONE, GL_ZERO);

    glVertexPointer(3, GL_FLOAT, 0, dynamic_meshes.vertex_array.data());
    glTexCoordPointer(2, GL_FLOAT, 0, dynamic_meshes.texture_coords_array.data());

    assert(implicit_surfaces.size() <= UINT_MAX);
    const auto implicit_surface_count = static_cast<unsigned int>(implicit_surfaces.size());
    for(auto ix = 0u; ix < implicit_surface_count; ++ix)
    {
        glBindTexture(GL_TEXTURE_2D, implicit_surfaces[ix].texture_id);

        assert(dynamic_meshes.index_array.size() < INT_MAX);    // GLsizei == int
        glDrawElements(GL_TRIANGLES,
                       dynamic_meshes.implicit_surfaces[ix].patch_count * dynamic_meshes.implicit_surfaces[ix].patch_count * 6,
                       GL_UNSIGNED_SHORT,
                       &dynamic_meshes.index_array[dynamic_meshes.implicit_surfaces[ix].index_array_offset]);
    }
}

#if 0
static void draw_patch(const Patch& patch, const Camera& camera)
{
    initialize_default_projection_matrix();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_ONE, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, patch.texture_id);

    glVertexPointer(3, GL_FLOAT, 0, patch.vertex_array.data());
    glTexCoordPointer(2, GL_FLOAT, 0, patch.texture_coords_array.data());

    assert(patch.index_array.size() < INT_MAX);   // GLsizei == int
    // TODO: 2016: Patch should pass an offset?  Offset will always be zero, since patch is the thing that hold the arrays.
    //             Either patch holds the arrays, and no offset, or it holds a reference to arrays, and have an offset.
    glDrawElements(GL_TRIANGLES, patch.patch_count * patch.patch_count * 6, GL_UNSIGNED_SHORT, &patch.index_array[patch.index_array_offset]);
}
#endif

// TODO: 2016: This isn't a true billboard, in the sense that it's only billboarded against the vertical axis.
// TODO: 2016: Pass in a Billboard object, with center_position, vertices, texture (ID and coords).
// TODO: 2016: Precalculate a full vertex array for the Emitter.  Texture and index array will be static.  Then,
// compare the draw_billboard function with draw_patch, and see if there are any material differences left.
static void draw_billboard(const Camera& camera, const Vector3f& position, float size, unsigned int texture_id)
{
    // Vertices are specified in left-to-right order from upper-left corner.
    const Vector3f vertex_array[]{{ -size,  size, 0.0f },
                                  {  size,  size, 0.0f },
                                  { -size, -size, 0.0f },
                                  {  size, -size, 0.0f }};

    // TODO: 2016: Like below, this texture_coords format could be generated, scaled, etc.
    constexpr Vector2f texture_coords_array[]{{ 0.0f, 0.0f },
                                              { 1.0f, 0.0f },
                                              { 0.0f, 1.0f },
                                              { 1.0f, 1.0f }};

    // TODO: 2016: With billboards, patches, and world geometry in the same format, there
    // may be a helper function that can generate the index_array for either.
    // And its name IS JOHN CENA^H^H^H^H^H^H^H^H^Hgenerate_implicit_surface_index_array.
    // TODO: 2016: This should also be applied to draw_sprite.
    constexpr uint16_t index_array[]{ 0, 2, 1, 1, 2, 3 };

    // Transform to location.
    // GL_MODELVIEW assumed.
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());
    glTranslatef(position.x(), position.y(), position.z());

    // Billboard the sprite.
    glRotatef(-camera.m_degrees, 0.0f, 1.0f, 0.0f);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexPointer(3, GL_FLOAT, 0, &vertex_array[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &texture_coords_array[0]);

    glDrawElements(GL_TRIANGLES, ARRAYSIZE(index_array), GL_UNSIGNED_SHORT, index_array);
}

// TODO: 2014: Drawing should be done against a vertex/index array.
static void draw_emitter(const Emitter& emitter, const Camera& camera)
{
    initialize_default_projection_matrix();

    glMatrixMode(GL_MODELVIEW);

    // TODO: 2016: Particles need to be sorted by Z to blend correctly.
    std::for_each(emitter.cbegin(), emitter.cend(), [&](const Particle& particle)
    {
        // TODO: 2016: size should be a member of Emitter.
        draw_billboard(camera, particle.position, 0.25f, emitter.texture_id());
    });
}

static void draw_sprite_at_position(const ImageProcessing::Bitmap& bitmap, unsigned int texture_id, unsigned short x_position, unsigned short y_position)
{
    const float float_xsize = static_cast<float>(bitmap.xsize);
    const float float_ysize = static_cast<float>(bitmap.ysize);
    // Vertices are specified in left-to-right order from upper-left corner.
    const Vector3f vertex_array[]{{ 0.0f,        float_ysize, 0.0f },
                                  { float_xsize, float_ysize, 0.0f },
                                  { 0.0f,               0.0f, 0.0f },
                                  { float_xsize,        0.0f, 0.0f }};

    // TODO: 2016: Like below, this texture_coords format could be generated, scaled, etc.
    constexpr Vector2f texture_coords_array[]{{ 0.0f, 0.0f },
                                              { 1.0f, 0.0f },
                                              { 0.0f, 1.0f },
                                              { 1.0f, 1.0f }};

    // TODO: 2016: With billboards, patches, and world geometry in the same format, there
    // may be a helper function that can generate the index_array for either.
    // And its name IS JOHN CENA^H^H^H^H^H^H^H^H^Hgenerate_implicit_surface_index_array.
    constexpr uint16_t index_array[]{ 0, 2, 1, 1, 2, 3 };

    // TODO: 2016: Figure out something better than 640x480.  At the least, it should use the same
    // dimensions as glViewport.  There should be both resolution independent and resolution
    // dependent drawing options.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 784, 0, 561, -1, 1);

    // Project into the world.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(x_position, y_position, 0.0f);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDepthFunc(GL_ALWAYS);     // TODO: 2016: Turn off depth testing.
    glBlendFunc(GL_ONE, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexPointer(3, GL_FLOAT, 0, &vertex_array[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &texture_coords_array[0]);

    glDrawElements(GL_TRIANGLES, ARRAYSIZE(index_array), GL_UNSIGNED_SHORT, index_array);
}

// TODO: modularize into separate functions
void draw_map(
    const Map& map,
    const Dynamic_meshes& dynamic_meshes,
    const Camera& camera)
{
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);

    // TODO: 2016: Each draw function should implement this  Move this to each draw function.
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    initialize_default_projection_matrix();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());

    glVertexPointer(3, GL_FLOAT, 0, map.vertex_array.data());
    glTexCoordPointer(2, GL_FLOAT, 0, map.texture_coords_array.data());

    // single loop multi pass textured lighting
    // this is done is one pass because of visibility
    // problems on a second light pass once the world is drawn
    for(unsigned int ii = 0; ii < map.world_mesh.size(); ++ii)
    {
        const Polygon* iter = &map.world_mesh[ii];

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDepthFunc(GL_LESS);
        glBlendFunc(GL_ONE, GL_ZERO);
        glBindTexture(GL_TEXTURE_2D, iter->texture_id);

        assert(iter->index_array.size() <= INT_MAX);
        glDrawElements(GL_TRIANGLES, static_cast<int>(iter->index_array.size()), GL_UNSIGNED_SHORT, iter->index_array.data());

        if(iter->lightmap_id == 0)
        {
            continue;
        }

        glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
        glDepthFunc(GL_EQUAL);
        glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, iter->lightmap_id);

        glDrawElements(GL_TRIANGLES, static_cast<int>(iter->index_array.size()), GL_UNSIGNED_SHORT, iter->index_array.data());
    }

    //(void)dynamic_meshes;
    draw_dynamic_meshes(map.implicit_surfaces, dynamic_meshes, camera);
    //(void)patch1;
    //(void)patch2;
    //draw_patch(patch1, camera);
    //draw_patch(patch2, camera);

    std::for_each(std::cbegin(map.emitters), std::cend(map.emitters), [&camera](const Emitter& emitter)
    {
        draw_emitter(emitter, camera);
    });

    draw_sprite_at_position(map.texture_list[2], 2, 128, 128);

    assert(glGetError() == GL_NO_ERROR);
}

}

