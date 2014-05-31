#include "PreCompile.h"
#include "particle.h"
#include "Camera.h"

const unsigned int MAX_PARTICLES = 50;

Particle::Particle() :
    m_life(0.0f),
    m_position(make_vector(0.0f, 0.0f, 0.0f)),
    m_velocity(make_vector(0.0f, 0.0f, 0.0f))
{
}

Particle::Particle(const Vector3f& position)
{
    m_position.x() = position.x() + ((float)rand() / float(RAND_MAX) / 2.0f) - 0.25f;
    m_position.y() = position.y();
    m_position.z() = position.z();
    m_velocity.x() = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
    m_velocity.y() = ((float)rand() / float(RAND_MAX)) / 10.0f;
    m_velocity.z() = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
    m_life = (((float)rand() / float(RAND_MAX)) * 20.0f);
}

bool Particle::isDead() const
{
    return !(m_life > 0.0f);
}

void Particle::Update(float elapsed_milliseconds)
{
    if(m_life > 0.0f)
    {
        m_life -= elapsed_milliseconds;
        m_position += m_velocity;
    }
}

Vector3f Particle::position() const
{
    return m_position;
}


Emitter::Emitter(const Vector3f& position) :
    m_particles(MAX_PARTICLES),
    m_position(position)
{
}

void Emitter::Update(float elapsed_milliseconds)
{
    // TODO: this should be SSE
    for(unsigned int index = 0; index < MAX_PARTICLES; ++index)
    {
        if(m_particles[index].isDead())
        {
            new(&m_particles[index]) Particle(m_position);
        }
        else
        {
            m_particles[index].Update(elapsed_milliseconds);
        }
    }
}

size_t Emitter::get_particle_count() const
{
    return m_particles.size();
}

Vector3f Emitter::get_particle_position(size_t index) const
{
    return m_particles[index].position();
}

