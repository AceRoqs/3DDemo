#include "PreCompile.h"
#include "LinearAlgebra.h"

namespace Demo
{

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

// Vector normalization (unitize).
Vector3f normalize(const Vector3f& v1)
{
    const float magnitude = sqrtf(dot(v1, v1));

    // Assume that vectors are not (0,0,0).
    assert(magnitude >= 0.0f);

    const Vector3f unit = v1 * (1.0f / magnitude);
    return unit;
}

// Vector dot-product.
float dot(const Vector3f& v1, const Vector3f& v2)
{
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

// Vector cross-product.
Vector3f cross(const Vector3f& v1, const Vector3f& v2)
{
    Vector3f normal = {v1.y() * v2.z() - v1.z() * v2.y(),
                       v1.z() * v2.x() - v1.x() * v2.z(),
                       v1.x() * v2.y() - v1.y() * v2.x()};

    return normal;
}

// Vector addition.
Vector3f operator+(const Vector3f& v1, const Vector3f& v2)
{
    Vector3f result = {v1.x() + v2.x(),
                       v1.y() + v2.y(),
                       v1.z() + v2.z()};

    return result;
}

Vector3f& operator+=(Vector3f& v1, const Vector3f& v2)
{
    v1.x() += v2.x();
    v1.y() += v2.y();
    v1.z() += v2.z();

    return v1;
}

// Vector subtraction.
Vector3f operator-(const Vector3f& v1, const Vector3f& v2)
{
    Vector3f result = {v1.x() - v2.x(),
                       v1.y() - v2.y(),
                       v1.z() - v2.z()};

    return result;
}

// Vector scale.
Vector3f operator*(const Vector3f& v1, float f1)
{
    Vector3f result = {v1.x() * f1,
                       v1.y() * f1,
                       v1.z() * f1};

    return result;
}

Vector3f operator*(float f1, const Vector3f& v1)
{
    return operator*(v1, f1);
}

Vector3f& operator*= (Vector3f& v1, float magnitude)
{
    v1.x() *= magnitude;
    v1.y() *= magnitude;
    v1.z() *= magnitude;

    return v1;
}

// Distance between two 3D points.
float point_distance(const Vector3f& p1, const Vector3f& p2)
{
    //return sqrtf(powf(p1.x() - p2.x(), 2.0f) + powf(p1.y() - p2.y(), 2.0f) + powf(p1.z() - p2.z(), 2.0f));
    Vector3f p3 = p2 - p1;
    return sqrtf(dot(p3, p3));
}

std::istream& operator>>(std::istream& input_stream, Vector3f& point)
{
    input_stream >> point.element[0];
    input_stream >> point.element[1];
    input_stream >> point.element[2];

    return input_stream;
}

}

