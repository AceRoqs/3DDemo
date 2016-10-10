#include "PreCompile.h"
#include "BitmapReader.h"       // Pick up forward declarations to ensure correctness.
#include <ImageProcessing/pcx.h>
#include <ImageProcessing/targa.h>
#include <ImageProcessing/PixMap.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/Wrappers.h>
#include <PortableRuntime/CheckException.h>

namespace Demo
{

// TODO: 2014: Previously, ImageProcessing had a dependency on WindowsCommon before this function was removed.
// Figure out how to handle this type of glue code, and where code like this should live.
// TODO: Ensure passed in file name is UTF-8.
ImageProcessing::Bitmap bitmap_from_file(_In_z_ const char* file_name)
{
    const auto file = WindowsCommon::create_file(file_name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
    const DWORD size = GetFileSize(file, nullptr);
    const auto read_complete = WindowsCommon::create_event(nullptr, true, false, nullptr);

    OVERLAPPED overlapped {};
    overlapped.hEvent = read_complete;

    // TODO: zero inits.
    // TODO: truncates size.
    std::vector<uint8_t> buffer(size);
    DWORD size_read;
    CHECK_EXCEPTION(!ReadFile(file, buffer.data(), size, &size_read, &overlapped), std::string("Failure to read: ") + file_name);
    const auto hr = WindowsCommon::hresult_from_last_error();
    if(hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
    {
        CHECK_EXCEPTION(FAILED(hr), u8"False success returned on overlapped read.");
        CHECK_HR(hr);
    }
    WaitForSingleObject(read_complete, INFINITE);

    if(ImageProcessing::is_pcx_file_name(file_name))
    {
        return ImageProcessing::decode_bitmap_from_pcx_memory(buffer.data(), size);
    }
    else if(ImageProcessing::is_tga_file_name(file_name))
    {
        return ImageProcessing::decode_bitmap_from_tga_memory(buffer.data(), size);
    }
    else
    {
        const bool is_ppm = ImageProcessing::is_pixmap_file_name(file_name);
        CHECK_EXCEPTION(is_ppm, std::string("Unsupported file type: ") + file_name);

        return ImageProcessing::decode_bitmap_from_pixmap_memory(buffer.data(), size);
    }
}

}

