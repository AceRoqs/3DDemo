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

// This code is fine, but it is currently unused.
#if 0
static void generate_grid_texture_rgb(
    _Out_writes_(xsize * ysize) uint8_t* bitmap,
    unsigned int xsize,
    unsigned int ysize) noexcept
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

