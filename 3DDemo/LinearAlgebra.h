#ifndef LINEARALGEBRA_H
#define LINEARALGEBRA_H

// TODO: namespace

struct Vector2f
{
    float element[2];
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

    // TODO: Add x/y/z union.  This is too verbose and error prone.
    float element[3];
};

#endif

