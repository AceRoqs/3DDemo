//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: Rendering routines
//=========================================================================
#include "PreCompile.h"
#include "render.h"
#include "particle.h"
#include "world.h"
#include "Bitmap.h"
#include "Camera.h"
#include "LinearAlgebra.h"

const unsigned int quadratic_bezier_control_point_count = 3;
struct bezier_patch
{
    unsigned int control_point_indices[quadratic_bezier_control_point_count * quadratic_bezier_control_point_count];
};

static const Vector3f bezier_control_points[] =
{
    { -2, 0, -10 },     // 0
    { -2, 0, -11 },     // 1
    { -3, 0, -11 },     // 2
    { -2,-1, -10 },     // 3
    { -2,-1, -11 },     // 4
    { -3,-1, -11 },     // 5
    { -2,-2, -10 },     // 6
    { -2,-2, -11 },     // 7
    { -3,-2, -11 },     // 8
    { -4, 0, -11 },     // 9
    { -4, 0, -10 },     // 10
    { -4,-1, -11 },     // 11
    { -4,-1, -10 },     // 12
    { -4,-2, -11 },     // 13
    { -4,-2, -10 },     // 14
};

static const bezier_patch patches[] =
{
    { 0,  1,  2,  3,  4,  5,  6,  7,  8 },
    { 2,  9, 10,  5, 11, 12,  8, 13, 14 },
};

// http://en.wikipedia.org/wiki/B%C3%A9zier_curve
// B0(t) = (1-t)^2
// B1(t) = 2(1-t)t
// B2(t) = t^2
// t = [0..1]
static float bezier_quadratic_basis(unsigned int index, float t)
{
    float basis;
    if(index == 0)
    {
        basis = (1.0f - t) * (1.0f - t);
    }
    else if(index == 1)
    {
        basis = 2.0f * (1.0f - t) * t;
    }
    else
    {
        assert(index == 2);
        basis = t * t;
    }

    return basis;
}

// TODO: 2014: It would make much more sense to do this in a compute shader to generate the data where they are used.
static std::vector<Vector3f> generate_quadratic_bezier_quads(const Vector3f* control_points, const bezier_patch& patch, unsigned int patch_count)
{
    const auto curve_vertex_count = patch_count + 1;

    // Q(u,v) = sum[i=0..2]sum[j=0..2] Bi(u)Bj(v)Pij
    std::vector<Vector3f> vertices(curve_vertex_count * curve_vertex_count);

    // Generate all of the points.
    for(unsigned int v = 0; v < curve_vertex_count; ++v)
    {
        for(unsigned int u = 0; u < curve_vertex_count; ++u)
        {
            Vector3f& vertex = vertices[v * curve_vertex_count + u];
            vertex = make_vector(0.0f, 0.0f, 0.0f);

            // Range [0..1].
            const float t_u = u / static_cast<float>(patch_count);
            const float t_v = v / static_cast<float>(patch_count);

            // Calculate the u,v point of the patch using three control points in each (u/v) direction.
            for(unsigned int j = 0; j < quadratic_bezier_control_point_count; ++j)
            {
                const float basis_v = bezier_quadratic_basis(j, t_v);

                for(unsigned int i = 0; i < quadratic_bezier_control_point_count; ++i)
                {
                    const float basis = bezier_quadratic_basis(i, t_u) * basis_v;

                    const size_t control_point_index = patch.control_point_indices[j * quadratic_bezier_control_point_count + i];
                    const Vector3f P = control_points[control_point_index];
                    vertex.x() += P.x() * basis;
                    vertex.y() += P.y() * basis;
                    vertex.z() += P.z() * basis;
                }
            }
        }
    }

    return vertices;
}

void draw_patch(const std::vector<Vector3f>& vertices, unsigned int patch_count, unsigned int texture_id)
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

            glTexCoord2f(k * scale, l * scale);
            glVertex3f(p1.x(), p1.y(), p1.z());

            glTexCoord2f((k + 1) * scale, l * scale);
            glVertex3f(p2.x(), p2.y(), p2.z());

            glTexCoord2f((k + 1) * scale, (l + 1) * scale);
            glVertex3f(p3.x(), p3.y(), p3.z());

            glTexCoord2f(k * scale, (l + 1) * scale);
            glVertex3f(p4.x(), p4.y(), p4.z());
        }
    }
    glEnd();
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
    const double FARCLIP = 700;
    glFrustum(LEFTCLIP, RIGHTCLIP, BOTTOMCLIP, TOPCLIP, NEARCLIP, FARCLIP);

    // Enable backface culling and hidden surface removal.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
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
    const std::vector<Graphics::Polygon>& poly_vector,
    const CEmitter& emitter,
    const Camera& camera)
{
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
//    glClearColor(0.0, 0.0, 0.0, 1.0);
//    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0, 1, 0);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());
    // first pass texturing
    glColor4f(1.0, 1.0, 1.0, 1.0);

    // single loop multi pass textured lighting
    // this is done is one pass because of visibility
    // problems on a second light pass once the world is drawn
    //    glLockArraysEXT(0, 43);
    for(unsigned int ii = 0; ii < poly_vector.size(); ii++)
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        const Graphics::Polygon* iter = &poly_vector[ii];
        glBindTexture(GL_TEXTURE_2D, iter->texture);
        glBlendFunc(GL_ONE, GL_ZERO);
        glDepthFunc(GL_LESS);

        // TODO: Just draw the whole world as one call
        unsigned char allIndices[4];
        // TODO11: remove the need for these casts.
        assert(poly_vector.size() * 4 < 256);
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
        glColor4f(0.0, 0.0, 0.0, 0.25f);
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

    // Set level-of-detail.
    const unsigned int MAX_GENERATED_POINTS = 10;
    unsigned int patch_count = (unsigned int)(MAX_GENERATED_POINTS * 4 / (point_distance(camera.m_position, make_vector(2, 0, 10)))) - 1;
    patch_count = std::min(std::max(2u, patch_count), MAX_GENERATED_POINTS - 1);
    std::vector<Vector3f> vertices = generate_quadratic_bezier_quads(bezier_control_points, patches[0], patch_count);
    draw_patch(vertices, patch_count, 2);
    vertices = generate_quadratic_bezier_quads(bezier_control_points, patches[1], patch_count);
    draw_patch(vertices, patch_count, 2);

//    glUnlockArraysEXT();
//  glDisable(GL_CULL_FACE);
    emitter.Draw(camera, 6);

    //swap_buffers();
#if 1
    //glFlush();
#else
    //glFinish();
#endif
}

