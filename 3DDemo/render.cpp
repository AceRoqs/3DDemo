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

struct bezier
{
    int ctl_pts[9];
};

const bezier patches[] =
{
    { 29, 30, 31, 32, 33, 34,  0, 35, 36 },
    { 31, 37, 38, 34, 39, 40, 36, 41, 42 }
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

static void BezCurve(const Camera& camera, const float* world_vector)
{
    const int PTS = 10;

    // Set level-of-detail.
    int lod = (int)(PTS * 4 / (point_distance(camera.m_position, make_vector(2, 0, 10))));

    lod = std::min(std::max(2, lod), PTS);

    for(size_t current_patch = 0; current_patch < ARRAYSIZE(patches); ++current_patch)
    {
        // Q(u,v) = sum[i=0..2]sum[j=0..2] Bi(u)Bj(v)Pij
        Vector3f pts[PTS][PTS]; // Q.

        // Generate all of the points.
        for(int v = 0; v < lod; ++v)
        {
            for(int u = 0; u < lod; ++u)
            {
                Vector3f& point = pts[u][v];
                point.x() = 0.0f;
                point.y() = 0.0f;
                point.z() = 0.0f;

                // Range [0..1].
                const float t_u = u / (lod - 1.0f);
                const float t_v = v / (lod - 1.0f);

                // Calculate the u,v point of the patch using three control points in each (u/v) direction.
                for(int j = 0; j < 3; ++j)
                {
                    const float basis_v = bezier_quadratic_basis(j, t_v);

                    for(int i = 0; i < 3; ++i)
                    {
                        const float basis = bezier_quadratic_basis(i, t_u) * basis_v;

                        const size_t world_vector_index = patches[current_patch].ctl_pts[j * 3 + i] * 3;
                        const float Px = world_vector[world_vector_index];
                        const float Py = world_vector[world_vector_index + 1];
                        const float Pz = world_vector[world_vector_index + 2];
                        point.x() += Px * basis;
                        point.y() += Py * basis;
                        point.z() += Pz * basis;
                    }
                }
            }
        }

        glBlendFunc(GL_ONE, GL_ZERO);
        glBindTexture(GL_TEXTURE_2D, 2);

        glBegin(GL_QUADS);
        for(int l = 0; l < lod - 1; ++l)
        {
            for(int k = 0; k < lod - 1; ++k)
            {
                // TODO: 2014: Make pts one dimensional, otherwise the drawing is compile time
                // tied to the size of the array.
                glTexCoord2f(k * 1.0f / lod, l * 1.0f / lod);
                glVertex3f(pts[k][l].x(), pts[k][l].y(), pts[k][l].z());
                glTexCoord2f((k+2) * 1.0f / lod , l * 1.0f / lod);
                glVertex3f(pts[k+1][l].x(), pts[k+1][l].y(), pts[k+1][l].z());
                glTexCoord2f((k+2) * 1.0f / lod, (l+2) * 1.0f / lod);
                glVertex3f(pts[k+1][l+1].x(), pts[k+1][l+1].y(), pts[k+1][l+1].z());
                glTexCoord2f(k * 1.0f / lod, (l+2) * 1.0f / lod);
                glVertex3f(pts[k][l+1].x(), pts[k][l+1].y(), pts[k][l+1].z());
            }
        }
        glEnd();
    } //  for current_patch
} // BezCurve

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
    const std::vector<Vector3f>& vertex_formats,
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
    glVertexPointer(3, GL_FLOAT, 0, &vertex_formats[0]);
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

    BezCurve(camera, g_WorldVector);

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

