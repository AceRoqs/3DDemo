#ifndef PCX_H
#define PCX_H

#include "Bitmap.h"

Bitmap decode_bitmap_from_pcx_memory(const uint8_t* file, size_t size);

bool PCXDecodeRGB(const char *filename, struct Bitmap*);

#endif

