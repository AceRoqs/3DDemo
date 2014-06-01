#include "PreCompile.h"
#include "particle.h"
#include "Camera.h"

const unsigned int MAX_PARTICLES = 50;

static bool is_particle_alive(const Particle& particle)
{
    return particle.life > 0.0f;
}

static Particle update_particle(Particle& particle, const Vector3f& emitter_position, float elapsed_milliseconds)
{
    if(is_particle_alive(particle))
    {
        particle.life -= elapsed_milliseconds;
        // TODO: 2014: Velocity update should be a function of elapsed_milliseconds.
        particle.position += particle.velocity;
    }
    else
    {
        // NOTE: If update_particle is moved back to a class, it may make sense to
        // make the generator a member, instead of re-seeding it on each update.
        std::random_device seed;
        std::default_random_engine generator(seed());
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

        // TODO: 2014: Clean this up.
        particle.position = emitter_position;
        particle.position.x() += distribution(generator) / 2.0f - 0.25f;

        particle.velocity.x() = 0.027f + distribution(generator) / -18.0f;
        particle.velocity.y() = distribution(generator) / 10.0f;
        particle.velocity.z() = 0.027f + distribution(generator) / -18.0f;

        particle.life = distribution(generator) * 20.0f;
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
        particle.life = 0.0f;
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

std::vector<Particle>::const_iterator Emitter::cbegin() const
{
    return m_particles.cbegin();
}

std::vector<Particle>::const_iterator Emitter::cend() const
{
    return m_particles.cend();
}

