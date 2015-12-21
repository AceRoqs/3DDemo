#pragma once

namespace ImageProcessing
{

#pragma pack(push)
#pragma pack(1)
struct Color_rgb
{
    // Empty default constructor is used to prevent zero-init when creating a std::vector<ColorRGB>.
    // These vectors are used as write targets for reads from disk, where zero-init would be inefficient.
    Color_rgb() {}

    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
#pragma pack(pop)

struct Bitmap
{
    Bitmap();

    std::vector<Color_rgb> bitmap;
    unsigned int xsize;
    unsigned int ysize;
    bool filtered;
};

}

