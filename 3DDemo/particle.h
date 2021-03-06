#pragma once

#include "LinearAlgebra.h"

namespace Demo
{

struct Particle_descriptor
{
    float position_x_scale;
    float position_x_bias;
    float position_y_scale;
    float position_y_bias;
    float position_z_scale;
    float position_z_bias;

    float velocity_x_scale;
    float velocity_x_bias;
    float velocity_y_scale;
    float velocity_y_bias;
    float velocity_z_scale;
    float velocity_z_bias;

    float life;
};

struct Particle
{
    float life_remaining_in_milliseconds;

    Vector3f position;
    Vector3f velocity;
};

class Emitter
{
    Particle_descriptor m_descriptor{};
    std::vector<Particle> m_particles{};
    Vector3f m_position{};
    unsigned int m_texture_index{};

public:
    Emitter(const Vector3f& emitter_position, unsigned int particle_count, const Particle_descriptor& descriptor, unsigned int texture_index);

    void update(float elapsed_milliseconds);

    unsigned int texture_index() const noexcept;

    std::vector<Particle>::const_iterator cbegin() const;
    std::vector<Particle>::const_iterator cend() const;
};

}

