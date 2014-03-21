//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: Implementation of PCX decoding routines
//=========================================================================
#include "PreCompile.h"
#include "pcx.h"
#include "block.h"

struct PCX
{
   int8_t  manufacturer;        // 10 = ZSoft PCX
   int8_t  version;             // 0 = v2.5 of PC Paintbrush
                                // 2 = v2.8 w/ palette info
                                // 3 = v2.8 w/o palette info
                                // 4 = PC Paintbrush for windows
                                // 5 = v3.0+ of PC Painbrush
   int8_t  encoding;            // 1 = RLE
   int8_t  bits_per_pixel;      // (per plane) 1, 2, 4, or 8
   int16_t xMin;
   int16_t yMin;
   int16_t xMax;
   int16_t yMax;
   int16_t HDpi;                // Horizontal resolution in dots per inch
   int16_t VDpi;                // Vertical resolution in dots per inch
   int8_t  colormap[48];
   int8_t  reserved;            // Should be 0
   int8_t  NPlanes;             // Number of color planes
   int16_t bytes_per_line;      // bytes per scan line plane, must be even
   int16_t palette_info;        // 1 = Color/BW, 2 = Grayscale
   int16_t Hscreen_size;        // Horizontal screen size in pixels
   int16_t Vscreen_size;        // Vertical screen size in pixels
   int8_t  filler[54];          // Padding to 128 bytes
};

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    char red;
    char green;
    char blue;
} ColorRGB;
#pragma pack(pop)

bool PCXDecodeRGB(const char* filename, block_t* spr)
{
    FILE* in;
    long total_size;
    int count, scanline;
    unsigned char buffer;
    PCX pcx;
    ColorRGB palette[256];

    int index, run_count;

    if(fopen_s(&in, filename, "rb") != 0)
    {
        return false;
    }
    if(in == nullptr)
    {
        return false;
    }
    if(fread(&pcx, 1, sizeof(pcx), in) == sizeof(pcx))
    {
        spr->xsize = pcx.xMax - pcx.xMin + 1;
        spr->ysize = pcx.yMax - pcx.yMin + 1;
        total_size = spr->xsize * spr->ysize * pcx.NPlanes * pcx.bits_per_pixel / 8;
        if(spr->bitmap != nullptr)
        {
            delete[] spr->bitmap;
        }
        spr->bitmap = new(std::nothrow) char[total_size];

        index = count = run_count = scanline = 0;
        do
        {
            if(fread(&buffer, 1, 1, in) != 1)
            {
                fclose(in);
                return false;
            }
            if(buffer > 192)
            {
                run_count = buffer & 63;
                fread(&buffer, 1, 1, in);
            }
            else
            {
                run_count = 1;
            }
            if(index + run_count > total_size)
            {
                return false;
            }
            for(int i = 0; i < run_count; ++i)
            {
                spr->bitmap[index + i] = buffer;
            }
            index += run_count;
            // we ignore the extra padding crap because I've never seen a
            // PCX with that padding
        } while(index < total_size);

        // translate palette crap

        fseek(in, -769, SEEK_END);

        fread(&buffer, 1, 1, in);
        if(fread(palette, 1, 768, in) == 768)
        {
            // read in palette
            ColorRGB *picture = new(std::nothrow) ColorRGB[spr->xsize * spr->ysize];
            for(long i = 0; i < spr->xsize * spr->ysize; ++i)
            {
                unsigned char x = spr->bitmap[i];

                picture[i].red = palette[x].red;
                picture[i].green = palette[x].green;
                picture[i].blue = palette[x].blue;
            }
            delete[] spr->bitmap;
            spr->bitmap = (char *)picture;
            fclose(in);
            return true;
        }
    }
    fclose(in);
    return false;
}

