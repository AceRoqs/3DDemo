#include "PreCompile.h"
#include "Bitmap.h"         // Pick up forward declarations to ensure correctness.
#include "pcx.h"
#include "targa.h"
#include <WindowsCommon/HRException.h>
#include <WindowsCommon/Wrappers.h>

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

// This code is fine, but it is currently unused.
#if 0
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
#endif

// This function is case sensitive due to the lack of library support for
// UTF-8 case insensitive matching.
static bool file_has_extension_case_sensitive(_In_z_ const char* file_name, _In_z_ const char* extension) NOEXCEPT
{
    const size_t length_file = strlen(file_name);
    const size_t length_extension = strlen(extension);
    return ((length_file >= length_extension) && (strcmp(file_name + length_file - length_extension, extension) == 0));
}

Bitmap bitmap_from_file(_In_z_ const char* file_name)
{
    const auto file = WindowsCommon::create_file(file_name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
    const DWORD size = GetFileSize(file, nullptr);
    const auto read_complete = WindowsCommon::create_event(nullptr, true, false, nullptr);

    OVERLAPPED overlapped = {};
    overlapped.hEvent = read_complete;

    // TODO: zero inits.
    // TODO: truncates size.
    std::vector<uint8_t> buffer(size);
    DWORD size_read;
    ReadFile(file, buffer.data(), size, &size_read, &overlapped);
    WaitForSingleObject(read_complete, INFINITE);

    if(file_has_extension_case_sensitive(file_name, ".pcx"))
    {
        return decode_bitmap_from_pcx_memory(buffer.data(), size);
    }
    else if(file_has_extension_case_sensitive(file_name, ".tga"))
    {
        return decode_bitmap_from_tga_memory(buffer.data(), size);
    }
    throw std::exception();
}

