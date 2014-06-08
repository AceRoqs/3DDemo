#ifndef PCX_H
#define PCX_H

#include "Bitmap.h"

Bitmap decode_bitmap_from_pcx_memory(_In_count_(size) const uint8_t* pcx_memory, size_t size);

bool PCXDecodeRGB(const char *filename, struct Bitmap*);

#endif

