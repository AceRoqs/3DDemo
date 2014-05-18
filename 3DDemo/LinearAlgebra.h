#ifndef LINEARALGEBRA_H
#define LINEARALGEBRA_H

// TODO: namespace

struct Vector2f
{
    float element[2];

    float& operator[](size_t ix)
    {
        return element[ix];
    }

    float& x()
    {
        return element[0];
    }

    float& y()
    {
        return element[1];
    }

    float x() const
    {
        return element[0];
    }

    float y() const
    {
        return element[1];
    }
};

struct Vector3f
{
    Vector3f()
    {
        // TODO: Zero init?
    }

    Vector3f(float x, float y, float z)
    {
        element[0] = x;
        element[1] = y;
        element[2] = z;
    }

    float element[3];

    float& operator[](size_t ix)
    {
        return element[ix];
    }

    float& x()
    {
        return element[0];
    }

    float& y()
    {
        return element[1];
    }

    float& z()
    {
        return element[2];
    }

    float x() const
    {
        return element[0];
    }

    float y() const
    {
        return element[1];
    }

    float z() const
    {
        return element[2];
    }
};

#endif

