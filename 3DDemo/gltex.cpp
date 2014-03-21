//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: OpenGL utility texture routines
//=========================================================================
#include "PreCompile.h"
#include "gltex.h"
#include "pcx.h"
#include "targa.h"
#include "block.h"

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

void bind_file_to_gl_texture(const char* filename, unsigned int texture_id)
{
    block_t block = { 0, 0, nullptr };

    bool use_default_texture;

    // TODO: 2014: Nice buffer read underrun!
    int cch = strlen(filename);
    if(strcmp(filename + cch - 4, ".pcx") == 0)
    {
        use_default_texture = !PCXDecodeRGB(filename, &block);
    }
    else if(strcmp(filename + cch - 4, ".tga") == 0)
    {
        use_default_texture = !TGADecodeRGB(filename, &block);
    }
    else
    {
        use_default_texture = true;
    }

    if(use_default_texture)
    {
        block.xsize  = 64;
        block.ysize  = 64;
        block.bitmap = new(std::nothrow) char[block.xsize * block.ysize * 3];
        generate_grid_texture_rgb(reinterpret_cast<unsigned char*>(block.bitmap), block.xsize, block.ysize);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if(use_default_texture)
    {
        // Don't filter the default texture.
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    else
    {
        // Bilinear filtering.
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 3,
                 block.xsize,
                 block.ysize,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 block.bitmap);

    delete[] block.bitmap;
}

