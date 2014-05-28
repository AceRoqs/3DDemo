#ifndef PARTICLE_H
#define PARTICLE_H

#include "LinearAlgebra.h"

struct color4_t
{
    float red, green, blue, alpha;
};

class CParticle
{
    Vector3f current_position;
    Vector3f previous_position;
    Vector3f current_velocity;
    unsigned int life;          // amount of time remaining
    color4_t color;             // current color
    color4_t final_color;       // color to fade to

public:
    CParticle();
    bool isDead() const;
    void Update(float elapsed_milliseconds);
    void Draw(const struct Camera& camera, unsigned int texture_id) const;

    friend class CEmitter;
};

class CEmitter
{
    Vector3f m_current_position;
    std::vector<CParticle> m_particles;

    void CreateParticle(unsigned int index);

public:
    CEmitter(const Vector3f& position);
    void Update(float elapsed_milliseconds);
    void Draw(const struct Camera& camera, unsigned int texture_id) const;
};

#endif

