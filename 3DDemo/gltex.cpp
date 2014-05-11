//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: OpenGL utility texture routines
//=========================================================================
#include "PreCompile.h"
#include "gltex.h"
#include "pcx.h"
#include "targa.h"
#include "Bitmap.h"

static void generate_grid_texture_rgb(unsigned char* bitmap, int xsize, int ysize)
{
    for(auto iy = 0; iy < ysize; ++iy)
    {
        for(auto ix = 0; ix < xsize; ++ix)
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

static void bind_bitmap_to_gl_texture(const Bitmap& bitmap, unsigned int texture_id)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if(bitmap.filtered)
    {
        // Bilinear filtering.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        // Don't filter the texture.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 3,
                 bitmap.xsize,
                 bitmap.ysize,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 &bitmap.bitmap[0]);
}

static Bitmap bitmap_from_file(const char* file_name)
{
    Bitmap bitmap;

    bool use_default_texture = true;
    const int cch = strlen(file_name);
    if((cch >= 4) && (strcmp(file_name + cch - 4, ".pcx") == 0))
    {
        use_default_texture = !PCXDecodeRGB(file_name, &bitmap);
    }
    else if((cch >= 4) && (strcmp(file_name + cch - 4, ".tga") == 0))
    {
        use_default_texture = !TGADecodeRGB(file_name, &bitmap);
    }

    if(use_default_texture)
    {
        bitmap.xsize  = 64;
        bitmap.ysize  = 64;
        bitmap.filtered = false;
        bitmap.bitmap.reset(new uint8_t[bitmap.xsize * bitmap.ysize * 3]);
        generate_grid_texture_rgb(reinterpret_cast<unsigned char*>(&bitmap.bitmap[0]), bitmap.xsize, bitmap.ysize);
    }

    return bitmap;
}

void bind_file_to_gl_texture(const char* file_name, unsigned int texture_id)
{
    Bitmap bitmap = bitmap_from_file(file_name);
    bind_bitmap_to_gl_texture(bitmap, texture_id);
}

