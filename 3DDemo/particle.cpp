#include "PreCompile.h"
#include "particle.h"
#include "Camera.h"

const unsigned int MAX_PARTICLES = 50;

CParticle::CParticle() : life(0.0f)
{
    current_position = make_vector(0.0f, 0.0f, 0.0f);
    current_velocity = make_vector(0.0f, 0.0f, 0.0f);
}

bool CParticle::isDead() const
{
    return !(life > 0.0f);
}

void CParticle::Update(float elapsed_milliseconds)
{
    life -= elapsed_milliseconds;

    if(life > 0.0f)
    {
        current_position += current_velocity;
    }
}

void CParticle::Draw(const Camera& camera, unsigned int texture_id) const
{
    // transform to location
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());
    glTranslatef(current_position.x(), current_position.y(), current_position.z());

    // billboard the sprite
    glRotatef(-camera.m_degrees, 0.0f, 1.0f, 0.0f);

    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, texture_id);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFunc(GL_ONE, GL_ZERO);
    glBlendFunc(GL_ONE, GL_ONE);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(0.25f, -0.25f, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-0.25f, -0.25f, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-0.25f, 0.25f, 0.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0.25f, 0.25f, 0.0f);
    glEnd();
}

CEmitter::CEmitter(const Vector3f& position) : m_particles(MAX_PARTICLES)
{
    m_current_position = position;
}

void CEmitter::CreateParticle(unsigned int index)
{
    m_particles[index].current_position.x() = m_current_position.x() +((float)rand() / float(RAND_MAX) / 2.0f) - 0.25f;
    m_particles[index].current_position.y() = m_current_position.y();
    m_particles[index].current_position.z() = m_current_position.z();
    m_particles[index].current_velocity.x() = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
    m_particles[index].current_velocity.y() = ((float)rand() / float(RAND_MAX)) / 10.0f;
    m_particles[index].current_velocity.z() = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
    m_particles[index].life = (((float)rand() / float(RAND_MAX)) * 20.0f);
}

void CEmitter::Update(float elapsed_milliseconds)
{
    for(unsigned int ii = 0; ii < MAX_PARTICLES; ++ii)
    {
        m_particles[ii].Update(elapsed_milliseconds);
        if(m_particles[ii].isDead())
        {
            CreateParticle(ii);
        }
    }
}

void CEmitter::Draw(const Camera& camera, unsigned int texture_id) const
{
//    glLoadIdentity();
//    glRotatef(camera_degrees, 0.0f, 1.0f, 0.0f);
//    glTranslatef(camera_x, camera_y, camera_z);
//    glRotatef(-camera_degrees, 0.0f, 1.0f, 0.0f);
/*
float modelview[16];
    int i, j;

    // save the current modelview matrix
    glPushMatrix();

    // get the current modelview matrix
    glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

    // undo all rotations
    // beware all scaling is lost as well 
    for(i = 0; i < 3; i++)
        for(j = 0; j < 3; j++)
        {
            if (i == j)
                modelview[i * 4 + j] = 1.0f;
            else
                modelview[i * 4 + j] = 0.0f;
        }

    // set the modelview with no rotations
    glLoadMatrixf(modelview);
*/
    // TODO: this should be SSE
    for(unsigned int ii = 0; ii < MAX_PARTICLES; ++ii)
    {
        m_particles[ii].Draw(camera, texture_id);
    }
}

