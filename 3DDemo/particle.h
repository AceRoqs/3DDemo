//=========================================================================
// Copyright (c) 1999-2003 Toby Jones. All rights reserved.
// Purpose: CParticle class
//=========================================================================
#ifndef PARTICLE_H
#define PARTICLE_H

const unsigned int MAXPARTICLES = 50;

class CEmitter;

//-------------------------------------------------------------------------
struct color4_t
{
    float red, green, blue, alpha;
}; // struct color4_t
//-------------------------------------------------------------------------
class CParticle
{
    float cur_x, cur_y, cur_z;  // current position
    float pre_x, pre_y, pre_z;  // previous position
    float vel_x, vel_y, vel_z;  // current velocity
    unsigned int life;          // amount of time remaining
    color4_t color;             // current color
    color4_t final_color;       // color to fade to

public:
    CParticle();
    bool isDead() const;
    void Update();
    void Draw(float, float, float, float, int) const;

    friend class CEmitter;
}; // class CParticle
//-------------------------------------------------------------------------
class CEmitter
{
    float m_cur_x, m_cur_y, m_cur_z;
    CParticle m_particles[MAXPARTICLES];

    void CreateParticle(unsigned int);

public:
    CEmitter();
    void setPosition(float, float, float);
    void Update();
    void Draw(float, float, float, float camera_degrees, int id) const;
}; // class CEmitter
//-------------------------------------------------------------------------

#endif

