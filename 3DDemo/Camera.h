#ifndef CAMERA_H
#define CAMERA_H

// TODO: namespace

struct Camera
{
    Camera(float x, float y, float z, float degrees) : m_x(x), m_y(y), m_z(z), m_degrees(degrees)
    {
    }

    float m_x;
    float m_y;
    float m_z;
    float m_degrees;
};

#endif

