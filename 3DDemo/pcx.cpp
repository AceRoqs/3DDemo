#include "PreCompile.h"
#include "pcx.h"
#include "Bitmap.h"

static enum PCX_manufacturer { PCX_magic = 10 };
static enum PCX_version
{
    PC_Paintbrush_2_5 = 0,          // PC Paintbrush 2.5.
    PC_Paintbrush_2_8_palette = 2,  // PC Paintbrush 2.8 with palette info.
    PC_Paintbrush_2_8 = 3,          // PC Paintbrush 2.8 without palette info.
    PC_Paintbrush_Windows = 4,      // PC Paintbrush for Windows.
    PC_Paintbrush_3 = 5,            // PC Paintbrush 3.0+.
};
static enum PCX_encoding { RLE_encoding = 1 };

#pragma pack(push)
#pragma pack(1)
struct PCX_header
{
    uint8_t  manufacturer;              // PCX_manufacturer.
    uint8_t  version;                   // PCX_version.
    uint8_t  encoding;                  // PCX_encoding.
    uint8_t  bits_per_pixel;            // Bits per pixel per plane - 1, 2, 4, or 8.
    uint16_t min_x;                     // Minimum X value - usually zero.
    uint16_t min_y;                     // Minimum Y value - usually zero.
    uint16_t max_x;                     // Maximum X value.
    uint16_t max_y;                     // Maximum Y value.
    uint16_t horizontal_dpi;            // Horizontal resolution in dots per inch.
    uint16_t vertical_dpi;              // Vertical resolution in dots per inch.
    uint8_t  color_map[48];             // EGA color palette.
    uint8_t  reserved;                  // Should be zero.
    uint8_t  color_plane_count;         // Number of color planes.
    uint16_t bytes_per_line;            // bytes per scan line plane, must be even.
    uint16_t palette_info;              // 1 = Color/BW, 2 = Grayscale.
    uint16_t horizontal_screen_size;    // Horizontal screen size in pixels.
    uint16_t vertical_screen_size;      // Vertical screen size in pixels.
    uint8_t  filler[54];                // Padding to 128 bytes.
};
#pragma pack(pop)

#ifdef USE_NEW_READERS
static void validate_pcx_header(_In_ const PCX_header* header)
{
    if(header->manufacturer != PCX_magic)
    {
        throw std::exception();
    }

    if(header->encoding != RLE_encoding)
    {
        throw std::exception();
    }

    if(header->min_x >= header->max_x)
    {
        throw std::exception();
    }

    if(header->min_y >= header->max_y)
    {
        throw std::exception();
    }

    if((header->color_plane_count != 1) && (header->color_plane_count != 3))
    {
        throw std::exception();
    }

    if(header->bits_per_pixel != 8)
    {
        throw std::exception();
    }
}

static const uint8_t* rle_decode(
    _In_ const uint8_t* start_iterator,
    _In_ const uint8_t* end_iterator,
    _Out_ uint8_t* value,
    _Out_ uint8_t* run_count)
{
    if(start_iterator >= end_iterator)
    {
        throw std::exception();
    }

    *run_count = 1;
    if(*start_iterator >= 192)
    {
        *run_count = *start_iterator - 192;
        ++start_iterator;

        if(start_iterator >= end_iterator)
        {
            throw std::exception();
        }
    }

    *value = *start_iterator;
    ++start_iterator;
    return start_iterator;
}

// TODO: The parameters need some reworking.
static void pcx_decode(
    _In_ const uint8_t* start_iterator,
    _In_ const uint8_t* end_iterator,
    unsigned int uncompressed_size,
    Color_rgb* bitmap, // TEMP?
    _In_count_(256) const Color_rgb* palette)
{
    uint8_t* bitmapb = reinterpret_cast<uint8_t*>(bitmap);
    unsigned int running_size = 0;
    do
    {
        uint8_t value;
        uint8_t run_count;
        start_iterator = rle_decode(start_iterator, end_iterator, &value, &run_count);

        if(running_size + run_count > uncompressed_size)
        {
            throw std::exception();
        }

        if(palette != nullptr)
        {
            // MSVC complains that fill_n is insecure.
            //std::fill_n(&bitmap.bitmap[running_size], run_count, palette[*iterator]);
            for(unsigned int ii = 0; ii < run_count; ++ii)
            {
                bitmap[running_size + ii] = palette[value];
            }
        }
        else
        {
            for(unsigned int ii = 0; ii < run_count; ++ii)
            {
                bitmapb[running_size + ii] = value;
            }
        }

        running_size += run_count;
    } while(running_size < uncompressed_size);
}

Bitmap decode_bitmap_from_pcx_memory(_In_count_(size) const uint8_t* pcx_memory, size_t size)
{
    if(size < sizeof(PCX_header))
    {
        throw std::exception();
    }

    const PCX_header* header = reinterpret_cast<const PCX_header*>(pcx_memory);
    validate_pcx_header(header);

    //const Color_rgb* palette = reinterpret_cast<const Color_rgb*>(pcx_memory + size - sizeof(Color_rgb) * 256);
    //if(size < sizeof(PCX_header) + sizeof(Color_rgb) * 256)
    //{
        // TODO: need to support non-palette PCX.
    //    throw std::exception();
    //}
    const Color_rgb* palette = nullptr;
    if(header->version == PC_Paintbrush_3 && header->color_plane_count == 1)
    {
        palette = reinterpret_cast<const Color_rgb*>(pcx_memory + size - sizeof(Color_rgb) * 256);
    }
    // Validate C0
    // Validate bounds

    Bitmap bitmap;
    bitmap.xsize = static_cast<unsigned int>(header->max_x) - header->min_x + 1;
    bitmap.ysize = static_cast<unsigned int>(header->max_y) - header->min_y + 1;
    bitmap.filtered = true;

    // uncompressed_size does not account for palette expansion.
    //const unsigned int uncompressed_size = header->bytes_per_line * bitmap.ysize;
    const unsigned int uncompressed_size = bitmap.xsize * bitmap.ysize * header->color_plane_count;
    //bitmap.bitmap.reserve(palette != nullptr ? uncompressed_size * sizeof(Color_rgb) : uncompressed_size);
    bitmap.bitmap.resize(palette != nullptr ? uncompressed_size * sizeof(Color_rgb) : uncompressed_size);

    const uint8_t* iterator = pcx_memory + sizeof(PCX_header);

    pcx_decode(iterator, palette != nullptr ? reinterpret_cast<const uint8_t*>(palette) - 1 : iterator + size, uncompressed_size, &bitmap.bitmap[0], palette);

    return bitmap;
}
#else
bool PCXDecodeRGB(const char* filename, Bitmap* spr)
{
    FILE* in;
    long total_size;
    int count, scanline;
    unsigned char buffer;
    PCX_header pcx;
    Color_rgb palette[256];

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
        spr->xsize = pcx.max_x - pcx.min_x + 1;
        spr->ysize = pcx.max_y - pcx.min_y + 1;
        spr->filtered = true;
        total_size = spr->xsize * spr->ysize * pcx.color_plane_count * pcx.bits_per_pixel / 8;

        // TODO: 2014: Use smart pointer for file handle, so new can throw.
        spr->bitmap.reset(new(std::nothrow) uint8_t[total_size]);

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
            for(int ii = 0; ii < run_count; ++ii)
            {
                spr->bitmap[index + ii] = buffer;
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
            Color_rgb* picture = new(std::nothrow) Color_rgb[spr->xsize * spr->ysize];
            for(unsigned long ii = 0; ii < spr->xsize * spr->ysize; ++ii)
            {
                unsigned char x = spr->bitmap[ii];

                picture[ii].red = palette[x].red;
                picture[ii].green = palette[x].green;
                picture[ii].blue = palette[x].blue;
            }
            spr->bitmap.reset(reinterpret_cast<uint8_t *>(picture));
            fclose(in);
            return true;
        }
    }
    fclose(in);
    return false;
}
#endif

