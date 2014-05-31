#include "PreCompile.h"
#include "particle.h"
#include "Camera.h"

const unsigned int MAX_PARTICLES = 50;

CParticle::CParticle() :
    m_life(0.0f),
    m_position(make_vector(0.0f, 0.0f, 0.0f)),
    m_velocity(make_vector(0.0f, 0.0f, 0.0f))
{
}

CParticle::CParticle(const Vector3f& position)
{
    m_position.x() = position.x() + ((float)rand() / float(RAND_MAX) / 2.0f) - 0.25f;
    m_position.y() = position.y();
    m_position.z() = position.z();
    m_velocity.x() = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
    m_velocity.y() = ((float)rand() / float(RAND_MAX)) / 10.0f;
    m_velocity.z() = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
    m_life = (((float)rand() / float(RAND_MAX)) * 20.0f);
}

bool CParticle::isDead() const
{
    return !(m_life > 0.0f);
}

void CParticle::Update(float elapsed_milliseconds)
{
    if(m_life > 0.0f)
    {
        m_life -= elapsed_milliseconds;
        m_position += m_velocity;
    }
}

Vector3f CParticle::position() const
{
    return m_position;
}


static void draw_billboard(const Camera& camera, const Vector3f& position, float size, unsigned int texture_id)
{
    // transform to location
    glLoadIdentity();
    glRotatef(camera.m_degrees, 0.0f, 1.0f, 0.0f);
    glTranslatef(camera.m_position.x(), camera.m_position.y(), camera.m_position.z());
    glTranslatef(position.x(), position.y(), position.z());

    // billboard the sprite
    glRotatef(-camera.m_degrees, 0.0f, 1.0f, 0.0f);

    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, texture_id);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFunc(GL_ONE, GL_ZERO);
    glBlendFunc(GL_ONE, GL_ONE);
    glBegin(GL_QUADS);
    {
        const float half_size = size / 2.0f;

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(half_size, -half_size, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-half_size, -half_size, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-half_size, half_size, 0.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(half_size, half_size, 0.0f);
    }
    glEnd();
}

CEmitter::CEmitter(const Vector3f& position) :
    m_particles(MAX_PARTICLES),
    m_position(position)
{
}

void CEmitter::CreateParticle(unsigned int index)
{
    new(&m_particles[index]) CParticle(m_position);
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
    // TODO: this should be SSE
    for(unsigned int ii = 0; ii < m_particles.size(); ++ii)
    {
        draw_billboard(camera, m_particles[ii].position(), 0.5f, texture_id);
    }
}

