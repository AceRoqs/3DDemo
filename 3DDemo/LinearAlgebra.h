#pragma once

namespace Demo
{

struct Vector2f
{
    float element[2];

    float& operator[](size_t ix);
    float& x();
    float& y();

    float x() const;
    float y() const;
};

struct Vector3f
{
    float element[3];

    float& operator[](size_t ix);
    float& x();
    float& y();
    float& z();

    float x() const;
    float y() const;
    float z() const;
};

Vector3f normalize(const Vector3f& vector);
float dot(const Vector3f& v1, const Vector3f& v2);
Vector3f cross(const Vector3f& v1, const Vector3f& v2);
Vector3f operator+(const Vector3f& v1, const Vector3f& v2);
Vector3f& operator+=(Vector3f& v1, const Vector3f& v2);
Vector3f operator-(const Vector3f& v1, const Vector3f& v2);
Vector3f operator*(const Vector3f& v1, float f1);
Vector3f& operator*= (Vector3f& v1, float magnitude);

float point_distance(const Vector3f& p1, const Vector3f& p2);

std::istream& operator>>(std::istream& input_stream, Vector3f& point);

}

