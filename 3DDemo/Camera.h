#pragma once

// TODO: namespace

#include "LinearAlgebra.h"

struct Camera
{
    Camera(const Vector3f& position, float degrees) : m_position(position), m_degrees(degrees)
    {
    }

    Vector3f m_position;
    float m_degrees;
};

