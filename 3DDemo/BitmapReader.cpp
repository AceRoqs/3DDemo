#include "PreCompile.h"
#include "BitmapReader.h"       // Pick up forward declarations to ensure correctness.
#include <ImageProcessing/pcx.h>
#include <ImageProcessing/targa.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/Wrappers.h>
#include <PortableRuntime/CheckException.h>

namespace Demo
{

// This function is case sensitive due to the lack of library support for
// UTF-8 case insensitive matching.
// TODO: Consider case insensitive for ASCII subset of UTF-8.
static bool file_has_extension_case_sensitive(_In_z_ const char* file_name, _In_z_ const char* extension) noexcept
{
    const size_t length_file = strlen(file_name);
    const size_t length_extension = strlen(extension);
    return ((length_file >= length_extension) && (strcmp(file_name + length_file - length_extension, extension) == 0));
}

// TODO: 2014: Previously, ImageProcessing had a dependency on WindowsCommon before this function was removed.
// Figure out how to handle this type of glue code, and where code like this should live.
ImageProcessing::Bitmap bitmap_from_file(_In_z_ const char* file_name)
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
    PortableRuntime::check_exception(!ReadFile(file, buffer.data(), size, &size_read, &overlapped));
    const HRESULT hr = WindowsCommon::hresult_from_last_error();
    WindowsCommon::check_with_custom_hr(hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING), hr);
    WaitForSingleObject(read_complete, INFINITE);

    if(file_has_extension_case_sensitive(file_name, ".pcx"))
    {
        return ImageProcessing::decode_bitmap_from_pcx_memory(buffer.data(), size);
    }
    else if(file_has_extension_case_sensitive(file_name, ".tga"))
    {
        return ImageProcessing::decode_bitmap_from_tga_memory(buffer.data(), size);
    }
    throw std::exception();     // TODO: Use check_exception instead of naked throw.
}

}

