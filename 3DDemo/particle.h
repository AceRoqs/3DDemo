#pragma once

#include "LinearAlgebra.h"

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
    Vector3f position;
    Vector3f velocity;
    float life;             // Amount of time remaining in milliseconds.
};

class Emitter
{
    Particle_descriptor m_descriptor;
    std::vector<Particle> m_particles;
    Vector3f m_position;

public:
    Emitter(const Vector3f& emitter_position, unsigned int particle_count, const Particle_descriptor& descriptor);
    void update(float elapsed_milliseconds);

    std::vector<Particle>::const_iterator cbegin() const;
    std::vector<Particle>::const_iterator cend() const;
};

