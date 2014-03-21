//=========================================================================
// Copyright (c) 1999 Toby Jones. All rights reserved.
// Purpose: CPolygon class
//=========================================================================
#ifndef POLYGON_H
#define POLYGON_H

class CPolygon
{
private:
    unsigned int num_points;
    unsigned int* points;       // indices into vector table
    unsigned int* st;           // indices into texture coordinate table
    unsigned int texture;       // texture number
    unsigned int lightmap;      // index into lightmap list

public:
    CPolygon();
    CPolygon(const CPolygon&);
    ~CPolygon();

    CPolygon& operator=(const CPolygon&);

    unsigned int getTexture() const;
    unsigned int getNumPoints() const;
    unsigned int getPoint(unsigned int) const;
    unsigned int getTexCoord(unsigned int) const;
    bool hasLightmap() const;
    unsigned int getLightmap() const;

    friend std::istream& operator>>(std::istream&, CPolygon&);
    friend std::ostream& operator<<(std::ostream&, CPolygon&);
};

#endif

