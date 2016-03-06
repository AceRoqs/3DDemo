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

    // TODO: 2016: GL_TEXTURE_ENV is not a loading property - it's a drawing property.  Move this.
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

static void draw_patch(const Patch& patch, const Camera& camera)
{
    // GL_MODELVIEW assumed.
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_ONE, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, patch.texture_id);

    glVertexPointer(3, GL_FLOAT, 0, &patch.vertices[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &patch.texture_coords[0]);

    assert(patch.index_array.size() < INT_MAX);   // GLsizei == int
    // TODO: 2016: Patch should pass an offset?  Offset will always be zero, since patch is the thing that hold the arrays.
    //             Either patch holds the arrays, and no offset, or it holds a reference to arrays, and have an offset.
    glDrawElements(GL_TRIANGLES, patch.patch_count * patch.patch_count * 6, GL_UNSIGNED_SHORT, &patch.index_array[patch.index_array_offset]);
}

// TODO: 2016: This isn't a true billboard, in the sense that it's only billboarded against the vertical axis.
// TODO: 2016: Pass in a Billboard object, with center_position, vertices, texture (ID and coords).
// TODO: 2016: Precalculate a full vertex array for the Emitter.  Texture and index array will be static.  Then,
// compare the draw_billboard function with draw_patch, and see if there are any material differences left.
static void draw_billboard(const Camera& camera, const Vector3f& position, float size, unsigned int texture_id)
{
    // Vertices are specified in left-to-right order from upper-left corner.
    const Vector3f vertices[] = {{ -size,  size, 0.0f },
                                 {  size,  size, 0.0f },
                                 { -size, -size, 0.0f },
                                 {  size, -size, 0.0f }};

    const Vector2f texture_coords[] = {{ 0.0f, 0.0f },
                                       { 1.0f, 0.0f },
                                       { 0.0f, 1.0f },
                                       { 1.0f, 1.0f }};

    // TODO: 2016: With billboards and patches in the same format, there
    // may be a helper function that can generate the index_array for either.
    const uint16_t index_array[] = { 0, 2, 1, 1, 2, 3 };

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
    // TODO: 2016: This will be glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), once textures are 32-bit. :(
    glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &texture_coords[0]);

    glDrawElements(GL_TRIANGLES, ARRAYSIZE(index_array), GL_UNSIGNED_SHORT, index_array);
}

// TODO: 2014: Drawing should be done against a vertex/index array.
static void draw_emitter(const Emitter& emitter, const Camera& camera, unsigned int texture_id)
{
    std::for_each(emitter.cbegin(), emitter.cend(), [&, texture_id](const Particle& particle)
    {
        // TODO: 2016: size should be a member of Emitter.
        draw_billboard(camera, particle.position, 0.25f, texture_id);
    });
}

// TODO: modularize into separate functions
void draw_map(
    const Map& map,
    const struct Camera& camera,
    const Patch& patch1,
    const Patch& patch2,
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
        assert(map.world_mesh.size() < 65536 / 4);

        const uint16_t index_array[] =
        {
            static_cast<uint16_t>(ii * 4),
            static_cast<uint16_t>(ii * 4 + 1),
            static_cast<uint16_t>(ii * 4 + 2),
            static_cast<uint16_t>(ii * 4 + 3)
        };

        const Polygon* iter = &map.world_mesh[ii];

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDepthFunc(GL_LESS);
        glBlendFunc(GL_ONE, GL_ZERO);
        glBindTexture(GL_TEXTURE_2D, iter->texture);

        // TODO: 2016: Make this GL_TRIANGLES.  This might require changes to the persisted formats?
        glDrawElements(GL_QUADS, ARRAYSIZE(index_array), GL_UNSIGNED_SHORT, index_array);

        if(iter->lightmap == 0)
        {
            continue;
        }

        glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
        glDepthFunc(GL_EQUAL);
        glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, iter->lightmap);

        glDrawElements(GL_QUADS, ARRAYSIZE(index_array), GL_UNSIGNED_SHORT, index_array);
    }

    draw_patch(patch1, camera);
    draw_patch(patch2, camera);

    draw_emitter(emitter, camera, 6);

    assert(glGetError() == GL_NO_ERROR);
}

}

