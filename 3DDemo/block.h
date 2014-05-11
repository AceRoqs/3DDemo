#ifndef BLOCK_H
#define BLOCK_H

struct block_t
{
    int xsize, ysize;
    std::unique_ptr<uint8_t[]> bitmap;
};

#endif

