#ifndef PARTICLE_H
#define PARTICLE_H

#include "LinearAlgebra.h"

class Particle
{
    Vector3f m_position;
    Vector3f m_velocity;
    float m_life;           // Amount of time remaining in milliseconds.

public:
    Particle();
    Particle(const Vector3f& position);
    bool isDead() const;
    void Update(float elapsed_milliseconds);

    Vector3f position() const;
};

class Emitter
{
    std::vector<Particle> m_particles;
    Vector3f m_position;

public:
    Emitter(const Vector3f& position);
    void Update(float elapsed_milliseconds);

    size_t get_particle_count() const;
    Vector3f get_particle_position(size_t index) const;
};

#endif

