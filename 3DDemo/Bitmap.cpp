#include "PreCompile.h"
#include "Bitmap.h"         // Pick up forward declarations to ensure correctness.
#include "pcx.h"
#include "targa.h"

namespace ImageProcessing
{

Bitmap::Bitmap() :
    xsize(0),
    ysize(0),
    filtered(false)
{
}

// http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
#if _MSC_VER <= 1800
// Add default and move constructors as VS2010/2012/2013 doesn't default define a move constructor.
Bitmap::Bitmap(Bitmap&& other) :
    bitmap(std::move(other.bitmap)),
    xsize(other.xsize),
    ysize(other.ysize),
    filtered(other.filtered)
{
}

Bitmap& Bitmap::operator=(Bitmap&& other) NOEXCEPT
{
    // Handle A=A case.
    if(this != &other)
    {
        bitmap = std::move(other.bitmap);
        xsize = other.xsize;
        ysize = other.ysize;
        filtered = other.filtered;
    }

    return *this;
}
#else
#error This compiler may autodefine the default move constructor.
#endif

// This code is fine, but it is currently unused.
#if 0
static void generate_grid_texture_rgb(
    _Out_writes_(xsize * ysize) uint8_t* bitmap,
    unsigned int xsize,
    unsigned int ysize) NOEXCEPT
{
    for(unsigned int iy = 0; iy < ysize; ++iy)
    {
        for(unsigned int ix = 0; ix < xsize; ++ix)
        {
            if((ix < (xsize / 2)) ^ (iy >= (ysize / 2)))
            {
                bitmap[0] = 0xc0;
                bitmap[1] = 0xc0;
                bitmap[2] = 0xc0;
            }
            else
            {
                bitmap[0] = 0xff;
                bitmap[1] = 0xff;
                bitmap[2] = 0xff;
            }

            bitmap += 3;
        }
    }
}
#endif

}

