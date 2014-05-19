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

#endif

