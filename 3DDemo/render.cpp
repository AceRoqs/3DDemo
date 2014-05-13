//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: Rendering routines
//=========================================================================
#include "PreCompile.h"
#include "render.h"
#include "gltex.h"
#include "particle.h"
#include "world.h"
#include "Bitmap.h"
#include "Camera.h"

struct bezier
{
    int ctl_pts[9];
};

const bezier columns[] =
{
    { 29, 30, 31, 32, 33, 34,  0, 35, 36 },
    { 31, 37, 38, 34, 39, 40, 36, 41, 42 }
};

static CEmitter emitter;

static void BezCurve(GLfloat camera_x, GLfloat camera_y, GLfloat camera_z);


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

    // enable backface culling and hidden surface removal
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
}

void initialize_gl_world_data(
    const std::vector<Bitmap>& texture_list,
    const std::vector<Position_vertex>& vertex_formats,
    const std::vector<TexCoord>& texture_coords)
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
    const Camera& camera)
{
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
//    glClearColor(0.0, 0.0, 0.0, 1.0);
//    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0, 1, 0);
    glTranslatef(camera.m_x, camera.m_y, camera.m_z);
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

    BezCurve(camera.m_x, camera.m_y, camera.m_z);

//    glUnlockArraysEXT();
//  glDisable(GL_CULL_FACE);
    emitter.setPosition(-3, 0, -10.5);
//    emitter.setPosition(0, 0, 0);
    emitter.Update();
    emitter.Draw(camera.m_x, camera.m_y, camera.m_z, camera.m_degrees, 6);

    //swap_buffers();
#if 1
    //glFlush();
#else
    //glFinish();
#endif
}

//---------------------------------------------------------------------------
// dist()
//---------------------------------------------------------------------------
//distance between camera and (-2,0,-10)
static float dist(
    GLfloat camera_x,
    GLfloat camera_y,
    GLfloat camera_z)
{
    return sqrt(pow(-2 + camera_x,2) + pow(camera_y,2) + pow(-10 + camera_z,2));
}

//---------------------------------------------------------------------------
// BezCurve()
//---------------------------------------------------------------------------
static void BezCurve(
    GLfloat camera_x,
    GLfloat camera_y,
    GLfloat camera_z)
{
    const unsigned int PTS = 10;
    Position_vertex pts[PTS][PTS];

    // set level-of-detail
    int lod;
    lod = (int)(PTS / (dist(camera_x, camera_y, camera_z) * 0.25f));

    if(lod < 2)
    {
        lod = 2;    // minimum level of detail
    }
    if(lod > PTS)
    {
        lod = PTS;  // maximum level of detail
    }

    for(int w = 0; w < 2; ++w)
    {
        for(int v = 0; v < lod; ++v)
        {
            for(int u = 0; u < lod; ++u)
            {
                pts[u][v].aVertex[0] = 0.0;
                pts[u][v].aVertex[1] = 0.0;
                pts[u][v].aVertex[2] = 0.0;
                // TODO: This loop could really be optimized
                for(int j = 0; j < 3; ++j)
                {
                    for(int k = 0; k < 3; ++k)
                    {
                        float px = g_WorldVector[(columns[w].ctl_pts[j * 3 + k])*3];
                        float py = g_WorldVector[(columns[w].ctl_pts[j * 3 + k])*3+1];
                        float pz = g_WorldVector[(columns[w].ctl_pts[j * 3 + k])*3+2];
                        float bezu, bezv;
                        if(j == 0)
                            bezv = (1.0f-(float(v)/(float(lod) - 1.0f))) * (1.0f-(float(v)/(float(lod) - 1.0f)));
                        else if(j == 1)
                            bezv = 2.0f * (1.0f-(float(v)/(float(lod) - 1.0f))) * (float(v) / (float(lod) - 1.0f));
                        else
                            bezv = (float(v) / (float(lod) - 1.0f)) * (float(v) / (float(lod) - 1.0f));
                        if(k == 0)
                            bezu = (1.0f-(float(u)/(float(lod) - 1.0f))) * (1.0f-(float(u)/(float(lod) - 1.0f)));
                        else if(k == 1)
                            bezu = 2.0f * (1.0f-(float(u)/(float(lod) - 1.0f))) * (float(u) / (float(lod) - 1.0f));
                        else
                            bezu = (float(u) / (float(lod) - 1.0f)) * (float(u) / (float(lod) - 1.0f));
                        pts[u][v].aVertex[0] += px * bezv * bezu;
                        pts[u][v].aVertex[1] += py * bezv * bezu;
                        pts[u][v].aVertex[2] += pz * bezv * bezu;
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
                glTexCoord2f(float(k) * 1.0f / (float)lod, (float)l * 1.0f/(float)lod);
                glVertex3f(pts[k][l].aVertex[0], pts[k][l].aVertex[1], pts[k][l].aVertex[2]);
                glTexCoord2f((float)(k+2) * 1.0f/(float)lod , (float)l*1.0f/(float)lod);
                glVertex3f(pts[k+1][l].aVertex[0], pts[k+1][l].aVertex[1], pts[k+1][l].aVertex[2]);
                glTexCoord2f((float)(k+2) * 1.0f/(float)lod, (float)(l+2) * 1.0f/(float)lod);
                glVertex3f(pts[k+1][l+1].aVertex[0], pts[k+1][l+1].aVertex[1], pts[k+1][l+1].aVertex[2]);
                glTexCoord2f((float)k * 1.0f/(float)lod, (float)(l+2) * 1.0f/(float)lod);
                glVertex3f(pts[k][l+1].aVertex[0], pts[k][l+1].aVertex[1], pts[k][l+1].aVertex[2]);
            }
        }

        glEnd();
    } //  for w
} // BezCurve

