#include "PreCompile.h"
#include "LinearAlgebra.h"

float& Vector2f::operator[](size_t ix)
{
    return element[ix];
}

float& Vector2f::x()
{
    return element[0];
}

float& Vector2f::y()
{
    return element[1];
}

float Vector2f::x() const
{
    return element[0];
}

float Vector2f::y() const
{
    return element[1];
}

float& Vector3f::operator[](size_t ix)
{
    return element[ix];
}

float& Vector3f::x()
{
    return element[0];
}

float& Vector3f::y()
{
    return element[1];
}

float& Vector3f::z()
{
    return element[2];
}

float Vector3f::x() const
{
    return element[0];
}

float Vector3f::y() const
{
    return element[1];
}

float Vector3f::z() const
{
    return element[2];
}

Vector3f make_vector(float x, float y, float z)
{
    Vector3f vector;
    vector.x() = x;
    vector.y() = y;
    vector.z() = z;

    return vector;
}

