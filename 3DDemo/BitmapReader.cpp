#include "PreCompile.h"
#include "BitmapReader.h"       // Pick up forward declarations to ensure correctness.
#include <ImageProcessing/pcx.h>
#include <ImageProcessing/targa.h>
#include <ImageProcessing/PixMap.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/Wrappers.h>
#include <PortableRuntime/CheckException.h>

#include "LinearAlgebra.h"
namespace Demo
{

bool intersects(const Vector3f& ray)
{
    constexpr float radius = 0.5f;
    constexpr float r2 = radius * radius;
    constexpr Vector3f circle = {0, 0, -2};
    constexpr Vector3f zero = {0, 0, 0};
    const Vector3f p = zero - circle;

    const Vector3f a = p - (ray * dot(p, ray));
    const float a2 = dot(a, a);
    return a2 <= r2;
}

ImageProcessing::Bitmap get_ray_traced_bitmap()
{
    int width = 512;
    int height = 512;
    float near_plane = -1.f;

    ImageProcessing::Bitmap bitmap;
    bitmap.height = height;
    bitmap.width = width;
    bitmap.filtered = true;
    bitmap.bitmap.resize(height * width * 3);

    for(int j = 0; j < height; ++j)
    {
        for(int i = 0; i < width; ++i)
        {
            // TODO: range is [-1, 1) instead of [-1,1].
            Vector3f ray = {static_cast<float>(i) / width * 2 - 1, static_cast<float>(j) / height * 2 - 1, near_plane};
            float dist = point_distance(ray, {0,0,0});

            ray[0] /= dist;
            ray[1] /= dist;
            ray[2] /= dist;

            if(intersects(ray))
            {
                bitmap.bitmap[j * width * 3 + i * 3 + 0] = 255;
                bitmap.bitmap[j * width * 3 + i * 3 + 1] = 0;
                bitmap.bitmap[j * width * 3 + i * 3 + 2] = 0;
            }
            else
            {
                bitmap.bitmap[j * width * 3 + i * 3 + 0] = 0;
                bitmap.bitmap[j * width * 3 + i * 3 + 1] = 0;
                bitmap.bitmap[j * width * 3 + i * 3 + 2] = 0;
            }
        }
    }

    return bitmap;
}

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
#if 1
        const bool is_ppm = ImageProcessing::is_pixmap_file_name(file_name);
        CHECK_EXCEPTION(is_ppm, std::string("Unsupported file type: ") + file_name);

        return ImageProcessing::decode_bitmap_from_pixmap_memory(buffer.data(), size);
#else
        // For testing.
        return get_ray_traced_bitmap();
#endif
    }
}

}

