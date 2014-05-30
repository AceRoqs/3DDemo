#ifndef LINEARALGEBRA_H
#define LINEARALGEBRA_H

// TODO: namespace

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

Vector3f make_vector(float x, float y, float z);

float point_distance(const Vector3f& p1, const Vector3f& p2);

Vector3f operator*(const Vector3f& v1, float f1);
Vector3f& operator+=(Vector3f& v1, const Vector3f& v2);

#endif

