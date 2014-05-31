#ifndef PARTICLE_H
#define PARTICLE_H

#include "LinearAlgebra.h"

struct Particle
{
    Vector3f position;
    Vector3f velocity;
    float life;             // Amount of time remaining in milliseconds.
};

class Emitter
{
    std::vector<Particle> m_particles;
    Vector3f m_position;

public:
    Emitter(const Vector3f& emitter_position);
    void update(float elapsed_milliseconds);

    std::vector<Particle>::const_iterator cbegin() const;
    std::vector<Particle>::const_iterator cend() const;
};

#endif

