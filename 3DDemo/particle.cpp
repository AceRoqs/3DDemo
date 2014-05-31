#include "PreCompile.h"
#include "particle.h"
#include "Camera.h"

const unsigned int MAX_PARTICLES = 50;

static bool is_particle_alive(const Particle& particle)
{
    return particle.m_life > 0.0f;
}

static Particle update_particle(Particle& particle, const Vector3f& emitter_position, float elapsed_milliseconds)
{
    if(is_particle_alive(particle))
    {
        particle.m_life -= elapsed_milliseconds;
        particle.m_position += particle.m_velocity;
    }
    else
    {
        particle.m_position.x() = emitter_position.x() + ((float)rand() / float(RAND_MAX) / 2.0f) - 0.25f;
        particle.m_position.y() = emitter_position.y();
        particle.m_position.z() = emitter_position.z();
        particle.m_velocity.x() = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
        particle.m_velocity.y() = ((float)rand() / float(RAND_MAX)) / 10.0f;
        particle.m_velocity.z() = 0.027f - ((float)rand() / float(RAND_MAX)) / 18.0f;
        particle.m_life = (((float)rand() / float(RAND_MAX)) * 20.0f);
    }

    return particle;
}

Emitter::Emitter(const Vector3f& position) :
    m_particles(MAX_PARTICLES),
    m_position(position)
{
    std::for_each(std::begin(m_particles), std::end(m_particles), [](Particle& particle)
    {
        // The rest of the structure is uninitialized.
        particle.m_life = 0.0f;
    });
}

void Emitter::update(float elapsed_milliseconds)
{
    const auto update = [&, elapsed_milliseconds](Particle& particle) -> Particle
    {
        return update_particle(particle, m_position, elapsed_milliseconds);
    };

    // TODO: 2014: Consider a SSE version.
    std::transform(std::begin(m_particles), std::end(m_particles), std::begin(m_particles), update);
}

size_t Emitter::get_particle_count() const
{
    return m_particles.size();
}

Vector3f Emitter::get_particle_position(size_t index) const
{
    return m_particles[index].m_position;
}

