#include "PreCompile.h"
#include "Bitmap.h"
#include "pcx.h"
#include "targa.h"

Bitmap::Bitmap() :
    xsize(0),
    ysize(0),
    filtered(false)
{
}

// http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
#if _MSC_VER == 1600 || _MSC_VER == 1700
// Add default and move constructors as VS2010/2012 doesn't default define a move constructor.
Bitmap::Bitmap(Bitmap&& other) :
    bitmap(std::move(other.bitmap)),
    xsize(other.xsize),
    ysize(other.ysize),
    filtered(other.filtered)
{
}

Bitmap& Bitmap::operator=(Bitmap&& other) NOEXCEPT
{
    // Handle A=A case.
    if(this != &other)
    {
        bitmap = std::move(other.bitmap);
        xsize = other.xsize;
        ysize = other.ysize;
        filtered = other.filtered;
    }

    return *this;
}
#else
#error This compiler may autodefine the default move constructor.
#endif

static void generate_grid_texture_rgb(
    _Out_cap_(xsize * ysize) uint8_t* bitmap,
    unsigned int xsize,
    unsigned int ysize) NOEXCEPT
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

Bitmap bitmap_from_file(_In_z_ const char* file_name)
{
#if 1
    const HANDLE file = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(file == INVALID_HANDLE_VALUE)
    {
        throw std::exception();
    }
    DWORD size = GetFileSize(file, nullptr);

    // TODO: zero inits.
    // TODO: truncates size.
    std::vector<uint8_t> buffer(size);
    ReadFile(file, buffer.data(), size, &size, nullptr);
    CloseHandle(file);

    const int cch = strlen(file_name);
    if((cch >= 4) && (strcmp(file_name + cch - 4, ".pcx") == 0))
    {
        return decode_bitmap_from_pcx_memory(buffer.data(), static_cast<size_t>(size));
    }
    else if((cch >= 4) && (strcmp(file_name + cch - 4, ".tga") == 0))
    {
        return decode_bitmap_from_tga_memory(buffer.data(), static_cast<size_t>(size));
    }
    throw std::exception();
#else
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
#endif
}

