#ifndef TARGA_H
#define TARGA_H

#include "Bitmap.h"

bool TGADecodeRGB(const char* szFileName, struct Bitmap* spr);
Bitmap decode_bitmap_from_tga_memory(const uint8_t* file, size_t size);

#endif

