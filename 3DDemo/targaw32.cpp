//=========================================================================
// Copyright (c) 2000-2003 Toby Jones. All rights reserved.
// Purpose: Targa manipulation routines
//=========================================================================
#include "PreCompile.h"
#include "targa.h"
#include "block.h"

#define TRUEVISION_TARGA "TRUEVISION-TARGA"

#pragma pack(push)
#pragma pack(1)

typedef struct
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
} targa_t;

typedef struct
{
    int32_t   dwExtensionFilePos;
    int32_t   dwDirectoryFilePos;
    int8_t    szID[18];
} targa_footer_t;

typedef struct
{
/*  int16_t   cEntries; */
    int16_t   eTag1;
    int32_t   dwPos1;
    int32_t   dwSize1;
/*  ... */
} targa_devdir_t;

typedef struct
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
} targa_extarea_t;

#pragma pack(pop)

#pragma warning(push, 4)
#pragma warning(disable:4127) // disable "conditional expression is constant"

//---------------------------------------------------------------------------
// NOTE: file pointer is moved
HRESULT TargaReadHeader(
    HANDLE hFile,
    targa_t* ptga)
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
                            sizeof(targa_t),
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
    targa_footer_t* pfooter)
{
    HRESULT hr;
    do
    {
        if(SetFilePointer(hFile, 0 - sizeof(targa_footer_t), nullptr, FILE_END) == 0xFFFFFFFF)
        {
            hr = GetLastError();
            break;
        }

        DWORD cbRead;   // WARNING: never checked!!!
        BOOL fOk = ReadFile(hFile,
                            pfooter,
                            sizeof(targa_footer_t),
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
    const targa_footer_t* pfooter,
    targa_extarea_t* pext)
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
                            sizeof(targa_extarea_t),
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
    const targa_footer_t* pfooter,
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
    targa_devdir_t* pentry)
{
    HRESULT hr;
    do
    {
        DWORD cbRead;   // WARNING: never checked!!!
        BOOL fOk = ReadFile(hFile,
                            pentry,
                            sizeof(targa_devdir_t),
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
    if(SetFilePointer(hFile, sizeof(targa_t), nullptr, FILE_BEGIN) == 0xFFFFFFFF)
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
    targa_t* ptga)
{
    LONG off = sizeof(targa_t) + ptga->cbIDfield;

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
    targa_t* ptga)
{
    LONG off = sizeof(targa_t) +
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
    targa_extarea_t* pext)
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
    targa_extarea_t* pext)
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
    targa_extarea_t* pext)
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
    block_t* spr)
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
    targa_t tga;
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

