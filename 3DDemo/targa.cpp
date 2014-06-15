#include "PreCompile.h"
#include "targa.h"
#include "Bitmap.h"

#define TRUEVISION_TARGA "TRUEVISION-TARGA"

#pragma pack(push)
#pragma pack(1)

struct TGA_header
{
    int8_t    cbIDfield;
    int8_t    eColorMapType;
    int8_t    eImageType;
    int16_t   wFirstColorMapEntry;
    int16_t   wColorMapLength;
    int8_t    cbColorMapEntrySize;
    int16_t   xOrigin;
    int16_t   yOrigin;
    int16_t   cxWidth;
    int16_t   cyHeight;
    int8_t    cBitsPerPixel;
    int8_t    eImageDescriptorBits;
};

struct TGA_footer
{
    int32_t   dwExtensionFilePos;
    int32_t   dwDirectoryFilePos;
    int8_t    szID[18];
};

struct TGA_dev_directory
{
/*  int16_t   cEntries; */
    int16_t   eTag1;
    int32_t   dwPos1;
    int32_t   dwSize1;
/*  ... */
};

struct TGA_extended_area
{
    int16_t   cbSize;
    int8_t    szAuthorName[41];
    int8_t    szAuthorComment1[81];
    int8_t    szAuthorComment2[81];
    int8_t    szAuthorComment3[81];
    int8_t    szAuthorComment4[81];
    int16_t   wCreationMonth;
    int16_t   wCreationDay;
    int16_t   wCreationYear;
    int16_t   wCreationHour;
    int16_t   wCreationMinute;
    int16_t   wCreationSecond;
    int8_t    szJobName[41];
    int16_t   wJobHours;
    int16_t   wJobMinutes;
    int16_t   wJobSeconds;
    int8_t    szProgramName[41];
    int16_t   wVersionTimes100;
    int8_t    bVersionLetter;
    int8_t    eBlue;
    int8_t    eGreen;
    int8_t    eRed;
    int8_t    eAlpha;
    int16_t   wPixelAspectNumerator;
    int16_t   wPixelAspectDenominator;
    int16_t   wGammaValueNumerator;
    int16_t   wGammaValueDenominator;
    int32_t   dwColorCorrectionFilePos;
    int32_t   dwThumbnailFilePos;
    int32_t   dwScanlineTableFilePos;
    int8_t    bAttributeType;
} ;

#pragma pack(pop)

#if 1
void validate_tga_header(_In_ const TGA_header* header)
{
    if(header->eImageType != 2 && header->eImageType != 10)
    {
        throw std::exception();
    }

    // TODO: validate all used fields.
}

size_t get_pixel_data_offset(_In_ const TGA_header* header)
{
    return sizeof(TGA_header) +
           header->cbIDfield +
           header->wColorMapLength * (header->cbColorMapEntrySize + 7) / 8;
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
    bitmap.xsize = header->cxWidth;
    bitmap.ysize = header->cyHeight;
    bitmap.filtered = true;

    // TODO: overflow.
    DWORD cbBitmap = header->cxWidth * header->cyHeight * (header->cBitsPerPixel + 7) / 8;
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
    TGA_extended_area* pext)
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
                            sizeof(TGA_extended_area),
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
    LONG off = sizeof(*ptga) + ptga->cbIDfield;

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
               ptga->cbIDfield +
               ptga->wColorMapLength * (ptga->cbColorMapEntrySize + 7) / 8;

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
    TGA_extended_area* pext)
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
    TGA_extended_area* pext)
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
    TGA_extended_area* pext)
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

    if(tga.eImageType != 2 && tga.eImageType != 10)
    {
        hr = E_FAIL;
        return false;
    }

    // Use targa info to finish building surface descriptor
    //ddsd.dwWidth  = tga.cxWidth;
    //ddsd.dwHeight = tga.cyHeight;
    spr->xsize = tga.cxWidth;
    spr->ysize = tga.cyHeight;
    spr->filtered = true;

    // Allocate a temporary bitmap from the stack
    DWORD cbBitmap = tga.cxWidth * tga.cyHeight * (tga.cBitsPerPixel + 7) / 8;
//    char *pBitmap = (char*)_alloca(cbBitmap);
    spr->bitmap.reset(new(std::nothrow) uint8_t[cbBitmap]);

    // Read targa data into temporary area
//#pragma message("TODO: Pipe through RLE and make sure we are a truecolor bitmap...")
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

