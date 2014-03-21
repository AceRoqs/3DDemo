//=========================================================================
// Copyright (c) 1999 Toby Jones. All rights reserved.
// Purpose: Polygon class
//=========================================================================
#include "PreCompile.h"
#include "polygon.h"

CPolygon::CPolygon()
{
    num_points = 0;
    points = st = nullptr;
    texture = 0;
}

CPolygon::CPolygon(const CPolygon& copy_from)
{
    points = st = nullptr;
    *this = copy_from;
}

CPolygon::~CPolygon()
{
    delete[] points;
    delete[] st;
}

CPolygon& CPolygon::operator=(const CPolygon& copy_from)
{
    if(this != &copy_from)
    {
        num_points = copy_from.num_points;
        delete[] points;
        delete[] st;
        points = st = nullptr;
        if(num_points)
        {
            points = new(std::nothrow) unsigned int[num_points];
            st = new(std::nothrow) unsigned int[num_points];
            for(unsigned int i = 0; i < num_points; ++i)
            {
                points[i] = copy_from.points[i];
                st[i] = copy_from.st[i];
            }
        }
        texture = copy_from.texture;
        lightmap = copy_from.lightmap;
    } // A == A
    return *this;
}

unsigned int CPolygon::getTexture() const
{
    return texture;
}

unsigned int CPolygon::getNumPoints() const
{
    return num_points;
}

unsigned int CPolygon::getPoint(unsigned int index) const
{
    assert(index < num_points);
    return points[index];
}

unsigned int CPolygon::getTexCoord(unsigned int index) const
{
    assert(index < num_points);
    return st[index];
}

bool CPolygon::hasLightmap() const
{
    return (lightmap != 0);
}

unsigned int CPolygon::getLightmap() const
{
    return lightmap;
}

std::istream& operator>>(std::istream& is, CPolygon& polygon)
{
    is >> polygon.num_points;
    delete[] polygon.points;
    delete[] polygon.st;
    if(polygon.num_points)
    {
        polygon.points = new(std::nothrow) unsigned int[polygon.num_points];
        polygon.st = new(std::nothrow) unsigned int[polygon.num_points];
        for(unsigned int i = 0; i < polygon.num_points; ++i)
        {
            is >> polygon.points[i];
        }
        for(unsigned int i = 0; i < polygon.num_points; ++i)
        {
            is >> polygon.st[i];
        }
    }
    else
    {
        polygon.points = polygon.st = nullptr;
    }
    is >> polygon.texture >> polygon.lightmap;
    return is;
}

std::ostream& operator<<(std::ostream& os, CPolygon& polygon)
{
    os << polygon.num_points;
    for(unsigned int i = 0; i < polygon.num_points; ++i)
    {
        os << polygon.points[i];
    }
    for(unsigned int i = 0; i < polygon.num_points; ++i)
    {
        os << polygon.st[i];
    }
    os << polygon.texture << polygon.lightmap << std::endl;
    return os;
}

