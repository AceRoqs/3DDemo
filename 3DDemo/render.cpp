#include "PreCompile.h"
#include "render.h"
#include "particle.h"
#include "world.h"
#include "Bezier.h"
#include "Bitmap.h"
#include "Camera.h"
#include "LinearAlgebra.h"

// Vertices is a two dimensional array of patch vertices.  generate_quadratic_bezier_quads() creates the expected output.
// TODO: 2014: This isn't very cache friendly, as the array isn't read in-order.  Vertex/Index arrays would also be an improvement.
static void draw_patch(const std::vector<Vector3f>& vertices, unsigned int patch_count, unsigned int texture_id)
{
    const float scale = 1.0f / patch_count;
    const auto curve_vertex_count = patch_count + 1;

    glBlendFunc(GL_ONE, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glBegin(GL_QUADS);
    for(unsigned int l = 0; l < patch_count; ++l)
    {
        for(unsigned int k = 0; k < patch_count; ++k)
        {
            const Vector3f& p1 = vertices[l * curve_vertex_count + k];
            const Vector3f& p2 = vertices[l * curve_vertex_count + k + 1];
            const Vector3f& p3 = vertices[(l + 1) * curve_vertex_count + k + 1];
            const Vector3f& p4 = vertices[(l + 1) * curve_vertex_count + k];

            // Begin in upper-left corner, and draw counterclockwise.
            glTexCoord2f(k * scale, l * scale);
            glVertex3f(p1.x(), p1.y(), p1.z());

            glTexCoord2f(k * scale, (l + 1) * scale);
            glVertex3f(p4.x(), p4.y(), p4.z());

            glTexCoord2f((k + 1) * scale, (l + 1) * scale);
            glVertex3f(p3.x(), p3.y(), p3.z());

            glTexCoord2f((k + 1) * scale, l * scale);
            glVertex3f(p2.x(), p2.y(), p2.z());
        }
    }
    glEnd();
}

static void draw_billboard(const Camera& camera, const Vector3f& position, float size, unsigned int texture_id)
{
    // Transform to location.
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());
    glTranslatef(position.x(), position.y(), position.z());

    // Billboard the sprite.
    glRotatef(-camera.m_degrees, 0.0f, 1.0f, 0.0f);

    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, texture_id);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFunc(GL_ONE, GL_ZERO);
    glBlendFunc(GL_ONE, GL_ONE);
    glBegin(GL_QUADS);
    {
        const float half_size = size / 2.0f;

        // Begin in upper-left corner, and draw counterclockwise.
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-half_size, half_size, 0.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-half_size, -half_size, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(half_size, -half_size, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(half_size, half_size, 0.0f);
    }
    glEnd();
}

// TODO: 2014: Drawing should be done against a vertex/index array.
static void draw_emitter(const Emitter& emitter, const Camera& camera, unsigned int texture_id)
{
    std::for_each(emitter.cbegin(), emitter.cend(), [&, texture_id](const Particle& particle)
    {
        draw_billboard(camera, particle.position, 0.5f, texture_id);
    });
}

static void bind_bitmap_to_gl_texture(const Bitmap& bitmap, unsigned int texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

    // Enable backface culling and hidden surface removal.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
}

void initialize_gl_world_data(
    const std::vector<Bitmap>& texture_list,
    const std::vector<Vector3f>& vertices,
    const std::vector<Vector2f>& texture_coords)
{
    // Load all texture data.
    for(size_t ix = 0; ix < texture_list.size(); ++ix)
    {
        bind_bitmap_to_gl_texture(texture_list[ix], ix);
    }

    // Enable vertex arrays.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &texture_coords[0]);
}

// TODO: add more flushes
// TODO: modularize into separate functions
void draw_list(
    const Map& map,
    const std::vector<Vector3f>& vertices,
    const std::vector<Vector3f>& vertices2,
    unsigned int patch_count,
    unsigned int patch_texture_id,
    const class Emitter& emitter,
    const struct Camera& camera)
{
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
//    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());
    // first pass texturing
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // single loop multi pass textured lighting
    // this is done is one pass because of visibility
    // problems on a second light pass once the world is drawn
    //    glLockArraysEXT(0, 43);
    for(unsigned int ii = 0; ii < map.world_mesh.size(); ii++)
    {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        const Graphics::Polygon* iter = &map.world_mesh[ii];
        glBindTexture(GL_TEXTURE_2D, iter->texture);
        glBlendFunc(GL_ONE, GL_ZERO);
        glDepthFunc(GL_LESS);

        // TODO: Just draw the whole world as one call
        unsigned char allIndices[4];
        // TODO11: remove the need for these casts.
        assert(map.world_mesh.size() * 4 < 256);
        allIndices[0] = static_cast<unsigned char>(ii * 4);
        allIndices[1] = static_cast<unsigned char>(ii * 4 + 1);
        allIndices[2] = static_cast<unsigned char>(ii * 4 + 2);
        allIndices[3] = static_cast<unsigned char>(ii * 4 + 3);
//        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, allIndices);
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, allIndices);
/*
        glBegin(GL_QUADS);
        {
            for(unsigned int i = 0; i < iter->getNumPoints(); ++i)
            {
                // TODO: texture coordinates should be an a texture array
//                glTexCoord2f(WorldTexture[iter->getTexCoord(i) * 2], WorldTexture[iter->getTexCoord(i) * 2 + 1]);
//                glArrayElement(iter->getPoint(i));
                glArrayElement(ii * 4 + i);
            }
        }
        glEnd();
*/
        if(iter->lightmap == 0)
        {
            continue;
        }

        glDepthFunc(GL_EQUAL);
        glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
        glBindTexture(GL_TEXTURE_2D, iter->lightmap);
//      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_ZERO, GL_SRC_ALPHA);

        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, allIndices);
//        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_BYTE, allIndices);
/*
        glBegin(GL_QUADS);
        {
            for(unsigned int i = 0; i < iter->getNumPoints(); ++i)
            {
                // TODO: texture coordinates should be an a texture array
//                glTexCoord2f(WorldTexture[iter->getTexCoord(i) * 2], WorldTexture[iter->getTexCoord(i) * 2 + 1]);
//                glArrayElement(iter->getPoint(i));
                glArrayElement(ii * 4 + i);
            }
        }
        glEnd();
*/
    }

    draw_patch(vertices, patch_count, patch_texture_id);
    draw_patch(vertices2, patch_count, patch_texture_id);

//    glUnlockArraysEXT();
//  glDisable(GL_CULL_FACE);
    draw_emitter(emitter, camera, 6);

    //swap_buffers();
#if 1
    //glFlush();
#else
    //glFinish();
#endif

    assert(glGetError() == GL_NO_ERROR);
}

