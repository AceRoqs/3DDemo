//=========================================================================
// Copyright (c) 1999 Toby Jones. All rights reserved.
// Purpose: CParticle class
//=========================================================================
#include "PreCompile.h"
#include "particle.h"
#include "Camera.h"

//-------------------------------------------------------------------------
CParticle::CParticle()
{
    cur_x = cur_y = cur_z = pre_x = pre_y = pre_z =	vel_x = vel_y = vel_z =	0.0f;
    life = 0;
    color.red = color.blue = color.green = 0.0;
    color.alpha = 1.0;
    final_color.red = final_color.blue = final_color.green = 0.0;
    final_color.alpha = 1.0;
} // CParticle::CParticle
//-------------------------------------------------------------------------
bool CParticle::isDead() const
{
    return (life == 0);
} // CParticle::isDead
//-------------------------------------------------------------------------
void CParticle::Update()
{
    if(life) --life;
    
    if(life)
    {
        pre_x = cur_x;
        pre_y = cur_y;
        pre_z = cur_z;

        cur_x += vel_x;
        cur_y += vel_y;
        cur_z += vel_z;
// TODO: update color
    }
} // CParticle::Update
//-------------------------------------------------------------------------
void CParticle::Draw(const Camera& camera, int id) const
{
    // transform to location
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0, 1, 0);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());
    glTranslatef(cur_x, cur_y, cur_z);

    // billboard the sprite
    glRotatef(-camera.m_degrees, 0, 1, 0);

//	glColor4f(color.red, color.green, color.blue, color.alpha);
    glColor4f(1,0,0,1);
    glBindTexture(GL_TEXTURE_2D, id);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_ONE, GL_ZERO);
    glBlendFunc(GL_ONE, GL_ONE);
    glBegin(GL_QUADS);
//	glBegin(GL_LINE_LOOP);
/*
        glTexCoord2f(0.0, 0.0);
        glVertex3d(cur_x + 0.25, cur_y - 0.25, cur_z);
        glTexCoord2f(1.0, 0.0);
        glVertex3d(cur_x - 0.25, cur_y - 0.25, cur_z);
        glTexCoord2f(1.0, 1.0);
        glVertex3d(cur_x - 0.25, cur_y + 0.25, cur_z);
        glTexCoord2f(0.0, 1.0);
        glVertex3d(cur_x + 0.25, cur_y + 0.25, cur_z);
*/
        glTexCoord2f(0.0, 0.0);
        glVertex3d(0.25, - 0.25, 0);
        glTexCoord2f(1.0, 0.0);
        glVertex3d(- 0.25, - 0.25, 0);
        glTexCoord2f(1.0, 1.0);
        glVertex3d(- 0.25, + 0.25, 0);
        glTexCoord2f(0.0, 1.0);
        glVertex3d(+ 0.25, + 0.25, 0);
    glEnd();
} // CParticle::Draw
//-------------------------------------------------------------------------
CEmitter::CEmitter()
{
    m_cur_x = m_cur_y = m_cur_z = 0.0;
} // CEmitter::CEmitter
//-------------------------------------------------------------------------
void CEmitter::CreateParticle(unsigned int index)
{
    m_particles[index].pre_x = m_particles[index].cur_x = m_cur_x +((float)rand() / float(RAND_MAX) / 2.0f) - 0.25f;
    m_particles[index].pre_y = m_particles[index].cur_y = m_cur_y;
    m_particles[index].pre_z = m_particles[index].cur_z = m_cur_z;
    m_particles[index].vel_x = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
    m_particles[index].vel_y = ((float)rand() / float(RAND_MAX)) / 10.0f;
    m_particles[index].vel_z = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
    m_particles[index].life = int(((float)rand() / float(RAND_MAX)) * 15.0);
    m_particles[index].color.red = 1.0;
    m_particles[index].color.green = 0.0;
    m_particles[index].color.blue = 0.0;
    m_particles[index].color.alpha = 1.0;
    m_particles[index].final_color.red = 1.0;
    m_particles[index].final_color.green = 0.0;
    m_particles[index].final_color.blue = 0.0;
    m_particles[index].final_color.alpha = 1.0;
} // CEmitter::CreateParticle
//-------------------------------------------------------------------------
void CEmitter::setPosition(float x, float y, float z)
{
    m_cur_x = x;
    m_cur_y = y;
    m_cur_z = z;
} // CEmitter::setPosition
//-------------------------------------------------------------------------
void CEmitter::Update()
{
    for(unsigned int i = 0; i < MAXPARTICLES; ++i)
    {
        m_particles[i].Update();
        if(m_particles[i].isDead())
            CreateParticle(i);
    }
} // CEmitter::Update
//-------------------------------------------------------------------------
void CEmitter::Draw(
    const Camera& camera,
    int id) const
{
//    glLoadIdentity();
//    glRotatef(camera_degrees, 0, 1, 0);
//    glTranslatef(camera_x, camera_y, camera_z);
//    glRotatef(-camera_degrees, 0, 1, 0);
/*
float modelview[16];
    int i, j;

    // save the current modelview matrix
    glPushMatrix();

    // get the current modelview matrix
    glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

    // undo all rotations
    // beware all scaling is lost as well 
    for( i=0; i<3; i++ ) 
        for( j=0; j<3; j++ ) {
        if ( i==j )
            modelview[i*4+j] = 1.0;
        else
            modelview[i*4+j] = 0.0;
        }

    // set the modelview with no rotations
    glLoadMatrixf(modelview);
*/
    // TODO: this should be SSE
    for(unsigned int i = 0; i < MAXPARTICLES; ++i)
    {
        m_particles[i].Draw(camera, id);
    }
} // CEmitter::Draw

