#include "PreCompile.h"
#include "targa.h"
#include "Bitmap.h"

#define TRUEVISION_TARGA "TRUEVISION-TARGA"

static enum TGA_color_map
{
    Has_no_color_map = 0,
    Has_color_map = 1,
};

static enum TGA_image_type
{
    No_image_data = 0,
    Color_mapped = 1,
    True_color = 2,
    Black_and_white = 3,
    RLE_color_mapped = 9,
    RLE_true_color = 10,
    RLE_black_and_white = 11,
};

#pragma pack(push)
#pragma pack(1)

struct TGA_header
{
    uint8_t  id_length;                 // Length of the image ID field.
    uint8_t  color_map_type;            // TGA_color_map.
    uint8_t  image_type;                // TGA_image_type.
    uint16_t color_map_first_index;     // Offset into color map table.
    uint16_t color_map_length;          // Number of entries.
    uint8_t  color_map_bits_per_pixel;  // Number of bits per pixel.
    uint16_t x_origin;                  // Lower-left corner.
    uint16_t y_origin;                  // Lower-left corner.
    uint16_t image_width;               // Width in pixels.
    uint16_t image_height;              // Height in pixels.
    uint8_t  bits_per_pixel;            // Bits per pixel.
    uint8_t  image_descriptor;          // Bits 0-3 are the alpha channel depth; bits 4-5 are the direction.
};

struct TGA_footer
{
    uint32_t extension_area_offset;     // Offset in bytes from beginning of file.
    uint32_t developer_area_offset;     // Offset in bytes from beginning of file.
    char     signature[18];             // "TRUEVISION-XFILE.".
};

struct TGA_dev_directory
{
/*  int16_t   cEntries; */
    int16_t   eTag1;
    int32_t   dwPos1;
    int32_t   dwSize1;
/*  ... */
};

struct TGA_extension_area
{
    uint16_t extension_size;            // Size in bytes of the extension area (always 495).
    char     author_name[41];           // Null terminated, space/null padded author name.
    char     author_comment1[81];       // Null terminated, space/null padded comment line 1.
    char     author_comment2[81];       // Null terminated, space/null padded comment line 2.
    char     author_comment3[81];       // Null terminated, space/null padded comment line 3.
    char     author_comment4[81];       // Null terminated, space/null padded comment line 4.
    uint16_t creation_month;
    uint16_t creation_day;
    uint16_t creation_year;
    uint16_t creation_hour;
    uint16_t creation_minute;
    uint16_t creation_second;
    char     job_name[41];              // Null terminated, space/null padded production job.
    uint16_t job_hours;                 // Time spent on the job (for billing).
    uint16_t job_minutes;
    uint16_t job_seconds;
    char     software_id[41];           // Null terminated, space/null padded creation application.
    uint16_t version;                   // Version number times 100.
    char     version_letter;
    uint8_t  blue;                      // Background color.  TODO: 2014: Should this be a Color_rgba?
    uint8_t  green;
    uint8_t  red;
    uint8_t  alpha;
    uint16_t pixel_aspect_numerator;    // Pixel aspect ratio.
    uint16_t pixel_aspect_denominator;
    uint16_t gamma_value_numerator;     // Gamma, in the range of 0.0-10.0.
    uint16_t gamma_value_denominator;   // Denominator of zero indicates field is unused.
    uint32_t color_correction_offset;   // Offset in bytes from beginning of file.
    uint32_t thumbnail_image_offset;    // Offset in bytes from beginning of file.
    uint32_t scanline_table_offset;     // Offset in bytes from beginning of file
    uint8_t  attributes_type;           // TODO: enum.
} ;

#pragma pack(pop)

#if 1
void validate_tga_header(_In_ const TGA_header* header)
{
    bool succeeded = true;

    succeeded &= (header->image_type == True_color);
    succeeded &= (header->bits_per_pixel == 24);
    succeeded &= (header->color_map_length == 0);
    succeeded &= (header->color_map_bits_per_pixel == 0);

    // image_height, image_width, and id_length are unbounded.

    if(!succeeded)
    {
        throw std::exception();
    }
}

size_t get_pixel_data_offset(_In_ const TGA_header* header)
{
    return sizeof(TGA_header) +
           header->id_length +
           header->color_map_length * (header->color_map_bits_per_pixel + 7) / 8;
}

Bitmap decode_bitmap_from_tga_memory(_In_count_(size) const uint8_t* tga_memory, size_t size)
{
    if(size < sizeof(TGA_header))
    {
        throw std::exception();
    }

    const TGA_header* header = reinterpret_cast<const TGA_header*>(tga_memory);
    validate_tga_header(header);

    Bitmap bitmap;
    bitmap.xsize = header->image_width;
    bitmap.ysize = header->image_height;
    bitmap.filtered = true;

    // TODO: overflow.
    DWORD cbBitmap = header->image_width * header->image_height * (header->bits_per_pixel + 7) / 8;
    const int pixel_count = cbBitmap / sizeof(Color_rgb);
    bitmap.bitmap.resize(pixel_count);

    // TODO: turn into function.
    size_t off = get_pixel_data_offset(header);

    // TODO: can be shorter.
    // TODO: prevent read overrun past size.

    // MSVC complains that std::copy is insecure.
    //std::copy(reinterpret_cast<const Color_rgb*>(&file[off]), reinterpret_cast<const Color_rgb*>(&file[off]) + pixel_count, &bitmap.bitmap[0]);
    memcpy(&bitmap.bitmap[0], tga_memory + off, pixel_count * sizeof(Color_rgb));

    return bitmap;
}

#else

#pragma warning(push, 4)
#pragma warning(disable:4127) // disable "conditional expression is constant"

//---------------------------------------------------------------------------
// NOTE: file pointer is moved
HRESULT TargaReadHeader(
    HANDLE hFile,
    TGA_header* ptga)
{
    HRESULT hr;
    do
    {
        if(SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) == 0xFFFFFFFF)
        {
            hr = GetLastError();
            break;
        }

        DWORD cbRead;   // WARNING: never checked!!!
        BOOL fOk = ReadFile(hFile,
                            ptga,
                            sizeof(*ptga),
                            &cbRead,
                            nullptr);

        if(!fOk)
        {
            hr = GetLastError();
            break;
        }

        hr = S_OK;
    } while(0);

    return hr;
} // TargaReadHeader
//---------------------------------------------------------------------------
// NOTE: file pointer is moved
HRESULT TargaReadFooter(
    HANDLE hFile,
    TGA_footer* pfooter)
{
    HRESULT hr;
    do
    {
        if(SetFilePointer(hFile, 0 - sizeof(TGA_footer), nullptr, FILE_END) == 0xFFFFFFFF)
        {
            hr = GetLastError();
            break;
        }

        DWORD cbRead;   // WARNING: never checked!!!
        BOOL fOk = ReadFile(hFile,
                            pfooter,
                            sizeof(TGA_footer),
                            &cbRead,
                            nullptr);

        if(!fOk)
        {
            hr = GetLastError();
            break;
        }

        hr = S_OK;
    } while(0);

    return hr;
} // TargaReadFooter
//---------------------------------------------------------------------------
// NOTE: file pointer is moved
HRESULT TargaReadExtArea(
    HANDLE hFile,
    const TGA_footer* pfooter,
    TGA_extension_area* pext)
{
    HRESULT hr;
    do
    {
        if(SetFilePointer(hFile, pfooter->dwExtensionFilePos, nullptr, FILE_BEGIN) == 0xFFFFFFFF)
        {
            hr = GetLastError();
            break;
        }

        DWORD cbRead;   // WARNING: never checked!!!
        BOOL fOk = ReadFile(hFile,
                            pext,
                            sizeof(TGA_extension_area),
                            &cbRead,
                            nullptr);

        if(!fOk)
        {
            hr = GetLastError();
            break;
        }

        hr = S_OK;
    } while(0);

    return hr;
} // TargaReadExtArea
//---------------------------------------------------------------------------
// NOTE: file pointer is moved
HRESULT TargaSeekToDevDir(
    HANDLE hFile,
    const TGA_footer* pfooter,
    WORD cEntries)
{
    HRESULT hr;
    do
    {
        if(SetFilePointer(hFile, pfooter->dwDirectoryFilePos, nullptr, FILE_BEGIN) == 0xFFFFFFFF)
        {
            hr = GetLastError();
            break;
        }

        DWORD cbRead;   // WARNING: never checked!!!
        BOOL fOk = ReadFile(hFile,
                            &cEntries,
                            sizeof(WORD),
                            &cbRead,
                            nullptr);

        if(!fOk)
        {
            hr = GetLastError();
            break;
        }

        hr = S_OK;
    } while(0);

    return hr;
} // TargaSeekToDevDir
//---------------------------------------------------------------------------
// NOTE: file pointer is moved
HRESULT TargaReadNextDevDirEntry(
    HANDLE hFile,
    TGA_dev_directory* pentry)
{
    HRESULT hr;
    do
    {
        DWORD cbRead;   // WARNING: never checked!!!
        BOOL fOk = ReadFile(hFile,
                            pentry,
                            sizeof(TGA_dev_directory),
                            &cbRead,
                            nullptr);

        if(!fOk)
        {
            hr = GetLastError();
            break;
        }

        hr = S_OK;
    } while(0);

    return hr;
} // TargaReadNextDevDirEntry
//---------------------------------------------------------------------------
HRESULT TargaSeekToID(
    HANDLE hFile)
{
    HRESULT hr;
    if(SetFilePointer(hFile, sizeof(TGA_header), nullptr, FILE_BEGIN) == 0xFFFFFFFF)
    {
        hr = GetLastError();
    }
    else
    {
        hr = S_OK;
    }

    return hr;
} // TargaSeekToID
//---------------------------------------------------------------------------
HRESULT TargaSeekToColorMap(
    HANDLE hFile,
    TGA_header* ptga)
{
    LONG off = sizeof(*ptga) + ptga->id_length;

    HRESULT hr;
    if(SetFilePointer(hFile, off, nullptr, FILE_BEGIN) == 0xFFFFFFFF)
    {
        hr = GetLastError();
    }
    else
    {
        hr = S_OK;
    }

    return hr;
} // TargaSeekToColorMap
//---------------------------------------------------------------------------
HRESULT TargaSeekToImage(
    HANDLE hFile,
    TGA_header* ptga)
{
    LONG off = sizeof(*ptga) +
               ptga->id_length +
               ptga->color_map_length * (ptga->color_map_bits_per_pixel + 7) / 8;

    HRESULT hr;
    if(SetFilePointer(hFile, off, nullptr, FILE_BEGIN) == 0xFFFFFFFF)
    {
        hr = GetLastError();
    }
    else
    {
        hr = S_OK;
    }

    return hr;
} // TargaSeekToImage
//---------------------------------------------------------------------------
HRESULT TargaSeekToColorCorrectionTable(
    HANDLE hFile,
    TGA_extension_area* pext)
{
    HRESULT hr;
    if(SetFilePointer(hFile, pext->dwColorCorrectionFilePos, nullptr, FILE_BEGIN) == 0xFFFFFFFF)
    {
        hr = GetLastError();
    }
    else
    {
        hr = S_OK;
    }

    return hr;
} // TargaSeekToColorCorrectionTable
//---------------------------------------------------------------------------
HRESULT TargaSeekToThumbnail(
    HANDLE hFile,
    TGA_extension_area* pext)
{
    HRESULT hr;
    if(SetFilePointer(hFile, pext->dwThumbnailFilePos, nullptr, FILE_BEGIN) == 0xFFFFFFFF)
    {
        hr = GetLastError();
    }
    else
    {
        hr = S_OK;
    }

    return hr;
} // TargaSeekToThumbnail
//---------------------------------------------------------------------------
HRESULT TargaSeekToScanlineTable(
    HANDLE hFile,
    TGA_extension_area* pext)
{
    HRESULT hr;
    if(SetFilePointer(hFile, pext->dwScanlineTableFilePos, nullptr, FILE_BEGIN) == 0xFFFFFFFF)
    {
        hr = GetLastError();
    }
    else
    {
        hr = S_OK;
    }

    return hr;
} // TargaSeekToScanlineTable
//---------------------------------------------------------------------------

bool TGADecodeRGB(
    const char* szFileName,
    Bitmap* spr)
{
    // Open the targa file
    HANDLE hFile;
    hFile = CreateFileA(szFileName,
                        GENERIC_READ,
                        0,
                        nullptr,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        nullptr);
    if(hFile == INVALID_HANDLE_VALUE)
        return false;

    // Read in targa info
    TGA_header tga;
    HRESULT hr = TargaReadHeader(hFile, &tga);
    if(SUCCEEDED(hr))
        hr = TargaSeekToImage(hFile, &tga);
    if(FAILED(hr))
        return false;

    if(tga.image_type != True_color)
    {
        hr = E_FAIL;
        return false;
    }

    // Use targa info to finish building surface descriptor
    //ddsd.dwWidth  = tga.cxWidth;
    //ddsd.dwHeight = tga.cyHeight;
    spr->xsize = tga.image_width;
    spr->ysize = tga.image_height;
    spr->filtered = true;

    // Allocate a temporary bitmap from the stack
    DWORD cbBitmap = tga.image_width * tga.image_height* (tga.bits_per_pixel + 7) / 8;
//    char *pBitmap = (char*)_alloca(cbBitmap);
    spr->bitmap.reset(new(std::nothrow) uint8_t[cbBitmap]);

    // Read targa data into temporary area
//#pragma message("TODO: Support transparent part of sprites using targa data")
    DWORD dwRead;
    ReadFile(hFile, &spr->bitmap[0], cbBitmap, &dwRead, nullptr);
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
/*
    // Create a compatible surface
    ddrval = IDirectX.lpDD->CreateSurface(&ddsd, &lpSurface, nullptr);
    if(ddrval != DD_OK)
        break;

    BOOL fBltOk = FALSE;

    // Lock the surface
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddrval = lpSurface->Lock(nullptr, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, nullptr);
    if(ddrval != DD_OK)
        break;

    // copy bitmap data to the actual surface
    switch(ddsdRear.ddpfPixelFormat.dwRGBBitCount)
    {
        // Remap the targa bitmap to a 24bpp bitmap
        case 24:
#ifdef USEASSEMBLY
            memcpy_dword(ddsd.lpSurface, pBitmap, cbBitmap);
#else
            memcpy(ddsd.lpSurface, pBitmap, cbBitmap);
#endif
            break;

        // Remap the targa bitmap to a 32bpp bitmap
        case 32:
        {
            int cPixels = tga.cxWidth * tga.cyHeight;
            for(int ix = 0; ix < cPixels; ix++)
            {
#pragma message("TODO: flip this the correct way...")
                ((PBGRA)ddsd.lpSurface)[ix].blue  = ((PBGR)pBitmap)[cPixels - 1 - ix].blue;
                ((PBGRA)ddsd.lpSurface)[ix].green = ((PBGR)pBitmap)[cPixels - 1 - ix].green;
                ((PBGRA)ddsd.lpSurface)[ix].red   = ((PBGR)pBitmap)[cPixels - 1 - ix].red;
                ((PBGRA)ddsd.lpSurface)[ix].alpha = 0xFF;
            }
            fBltOk = TRUE;
            break;
        } // case 32
        default:
        {
            TCHAR sz[64];

            _stprintf(sz,
                        _T("Display must be either %d or %d bits per pixel."),
                        DEFAULTBITDEPTH,
                        DEFAULTBITDEPTH - 8);
            MessageBox(IDirectX.hwnd,
                        sz,
                        _T("Unsupported color depth"),
                        MB_OK | MB_ICONWARNING);
        }
    } // switch
*/
    return true;
}

#pragma warning(pop)

#endif

