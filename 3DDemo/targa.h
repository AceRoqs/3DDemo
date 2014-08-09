#ifndef TARGA_H
#define TARGA_H

#include "Bitmap.h"

Bitmap decode_bitmap_from_tga_memory(_In_count_(size) const uint8_t* tga_memory, size_t size);
std::vector<uint8_t> encode_tga_from_bitmap(const Bitmap& bitmap);

#endif

