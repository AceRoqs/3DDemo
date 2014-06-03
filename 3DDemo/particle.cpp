#include "PreCompile.h"
#include "particle.h"
#include "Camera.h"

const unsigned int MAX_PARTICLES = 50;

static bool is_particle_alive(const Particle& particle)
{
    return particle.life > 0.0f;
}

static Particle update_particle(const Particle& current_particle, const Vector3f& emitter_position, float elapsed_milliseconds)
{
    Particle particle = current_particle;

    // Decrement life first.  If the elapsed_milliseconds is larger than the life, then the
    // calculated velocity and position will be significantly outside the expected bounds.
    particle.life -= elapsed_milliseconds;
    if(is_particle_alive(particle))
    {
        particle.position += particle.velocity * elapsed_milliseconds;
    }
    else
    {
        // NOTE: If update_particle is moved back to a class, it may make sense to
        // make the generator a member, instead of re-seeding it on each update.
        std::random_device seed;
        std::default_random_engine generator(seed());
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

        particle.position = emitter_position;
        particle.position.x() += distribution(generator) * 0.5f - 0.25f;

        particle.velocity.x() = distribution(generator) * -0.004f + 0.0016f;
        particle.velocity.y() = distribution(generator) *  0.008f + 0.0f;
        particle.velocity.z() = distribution(generator) * -0.004f + 0.0016f;

        particle.life = distribution(generator) * 150.0f;
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
    const auto update = [&, elapsed_milliseconds](const Particle& particle) -> Particle
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

