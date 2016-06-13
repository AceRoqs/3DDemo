#pragma once

#include "LinearAlgebra.h"

namespace Demo
{

struct Camera
{
    Vector3f m_position;

    // TODO: 2016: Document clockwise/counterclockwise, and what axis.
    // TODO: 2016: Be consistent on degrees/radians throughout.
    float m_degrees;
};

}

