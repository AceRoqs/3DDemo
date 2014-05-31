#ifndef PARTICLE_H
#define PARTICLE_H

#include "LinearAlgebra.h"

class CParticle
{
    Vector3f m_position;
    Vector3f m_velocity;
    float m_life;           // Amount of time remaining in milliseconds.

public:
    CParticle();
    CParticle(const Vector3f& position);
    bool isDead() const;
    void Update(float elapsed_milliseconds);

    Vector3f position() const;
};

class CEmitter
{
    std::vector<CParticle> m_particles;
    Vector3f m_position;

    void CreateParticle(unsigned int index);

public:
    CEmitter(const Vector3f& position);
    void Update(float elapsed_milliseconds);
    void Draw(const struct Camera& camera, unsigned int texture_id) const;
};

#endif

