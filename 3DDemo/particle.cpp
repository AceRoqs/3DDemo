#include "PreCompile.h"
#include "particle.h"
#include "Camera.h"

namespace Demo
{

static bool is_particle_alive(const Particle& particle) noexcept
{
    return particle.life_remaining_in_milliseconds > 0.0f;
}

static Particle update_particle(
    const Particle& current_particle,
    const Vector3f& emitter_position,
    const Particle_descriptor& descriptor,
    float elapsed_milliseconds)
{
    Particle particle = current_particle;

    // Decrement life first.  If the elapsed_milliseconds is larger than the life, then the
    // calculated velocity and position will be significantly outside the expected bounds.
    particle.life_remaining_in_milliseconds -= elapsed_milliseconds;
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
        particle.position.x() += distribution(generator) * descriptor.position_x_scale + descriptor.position_x_bias;
        particle.position.y() += distribution(generator) * descriptor.position_y_scale + descriptor.position_y_bias;
        particle.position.z() += distribution(generator) * descriptor.position_z_scale + descriptor.position_z_bias;

        particle.velocity.x() = distribution(generator) * descriptor.velocity_x_scale + descriptor.velocity_x_bias;
        particle.velocity.y() = distribution(generator) * descriptor.velocity_y_scale + descriptor.velocity_y_bias;
        particle.velocity.z() = distribution(generator) * descriptor.velocity_z_scale + descriptor.velocity_z_bias;

        particle.life_remaining_in_milliseconds = distribution(generator) * descriptor.life;
    }

    return particle;
}

static std::vector<Particle> generate_dead_particles(unsigned int particle_count)
{
    std::vector<Particle> particles;
    particles.reserve(particle_count);

    for(unsigned int ix = 0; ix < particle_count; ++ix)
    {
        // Only initialize life member variable.
        particles.push_back({0.0f});
    }

    return particles;
}

Emitter::Emitter(const Vector3f& position, unsigned int particle_count, const Particle_descriptor& descriptor, unsigned int texture_id) :
    m_descriptor(descriptor),
    m_particles(generate_dead_particles(particle_count)),
    m_position(position),
    m_texture_id(texture_id)
{
}

void Emitter::update(float elapsed_milliseconds)
{
    const auto update = [&, elapsed_milliseconds](const Particle& particle) -> Particle
    {
        return update_particle(particle, m_position, m_descriptor, elapsed_milliseconds);
    };

    // TODO: 2014: Consider a SSE version.
    std::transform(std::begin(m_particles), std::end(m_particles), std::begin(m_particles), update);
}

unsigned int Emitter::texture_id() const noexcept
{
    return m_texture_id;
}

std::vector<Particle>::const_iterator Emitter::cbegin() const
{
    return m_particles.cbegin();
}

std::vector<Particle>::const_iterator Emitter::cend() const
{
    return m_particles.cend();
}

}

