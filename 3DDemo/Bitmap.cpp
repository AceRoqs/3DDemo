#include "PreCompile.h"
#include "Bitmap.h"         // Pick up forward declarations to ensure correctness.
#include "pcx.h"
#include "targa.h"
#include "HRException.h"
#include "WindowClass.h"

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

Bitmap bitmap_from_file(_In_z_ const char* file_name)
{
    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if(overlapped.hEvent == nullptr)
    {
        WindowsCommon::throw_hr(WindowsCommon::hresult_from_last_error());
    }

    const auto file = WindowsCommon::create_file(file_name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
    if(file == INVALID_HANDLE_VALUE)
    {
        WindowsCommon::throw_hr(WindowsCommon::hresult_from_last_error());
    }
    DWORD size = GetFileSize(file, nullptr);

    // TODO: zero inits.
    // TODO: truncates size.
    std::vector<uint8_t> buffer(size);
    DWORD size_read;
    ReadFile(file, buffer.data(), size, &size_read, &overlapped);
    WaitForSingleObject(overlapped.hEvent, INFINITE);
    CloseHandle(overlapped.hEvent);

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
}

