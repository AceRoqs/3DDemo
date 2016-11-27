#include "PreCompile.h"
#include "BitmapReader.h"       // Pick up forward declarations to ensure correctness.
#include <ImageProcessing/Filter.h>
#include <ImageProcessing/pcx.h>
#include <ImageProcessing/targa.h>
#include <ImageProcessing/PixMap.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/Wrappers.h>
#include <PortableRuntime/CheckException.h>
#include <PortableRuntime/Tracing.h>

#include "LinearAlgebra.h"

namespace Demo
{

struct Sphere
{
    Vector3f center;
    float radius;
};

// Implements a non-optimized version of a ray-sphere intersection.
// TODO: 2016: Use a more optimal solution.
_Success_(return) bool ray_sphere_intersects(const Vector3f& ray_origin, const Vector3f& ray_direction, const Sphere& sphere, _Out_ Vector3f* intersection_point)
{
    // Real Time Rendering, 3rd edition has a good explaination of Ray/Sphere intersection.
    // t^2 + 2tb + c = 0
    // Rearranged:
    // t = -b +/- sqrt(b^2 - c)

    const float r2 = sphere.radius * sphere.radius;
    const Vector3f o_c = ray_origin - sphere.center;
    const float b = dot(ray_direction, o_c);
    const float c = dot(o_c, o_c) - r2;
    const float b2_c = b * b - c;

    if(b2_c >= 0.0f)
    {
        const float sqrt_b2_c = sqrt(b2_c);
        const float t = -b - sqrt_b2_c;
        assert(t < -b + sqrt_b2_c);

        *intersection_point = t * ray_direction;

        return true;
    }
    return false;
}

const Sphere* find_nearest_intersection(const Vector3f& ray_origin, const Vector3f& ray_direction, const std::vector<Sphere>& objects, _Out_ Vector3f* intersection_point)
{
    const Sphere* intersects = nullptr;

    for_each(std::cbegin(objects), std::cend(objects), [&](const Sphere& sphere)
    {
        Vector3f current_intersection_point;
        bool current_intersects = ray_sphere_intersects(ray_origin, ray_direction, sphere, &current_intersection_point);
        if(current_intersects)
        {
            if(intersects != nullptr)
            {
                if(dot(current_intersection_point, current_intersection_point) < dot(*intersection_point, *intersection_point))
                {
                    *intersection_point = current_intersection_point;
                }
            }
            else
            {
                intersects = &sphere;
                *intersection_point = current_intersection_point;
            }
        }
    });

    return intersects;
}

// Light vector and normal vector must be in the same coordinate space.
ImageProcessing::Color_rgb lambertian_shading_with_clamp(const Vector3f& light_vector, const Vector3f& normal_vector, ImageProcessing::Color_rgb material_color)
{
    const float intensity = std::max(dot(light_vector, normal_vector), 0.0f);

    ImageProcessing::Color_rgb color;
    color.red = static_cast<unsigned char>(material_color.red * intensity);
    color.green = static_cast<unsigned char>(material_color.green * intensity);
    color.blue = static_cast<unsigned char>(material_color.blue * intensity);

    return color;
}

// Input vectors must be in the same coordinate space.
ImageProcessing::Color_rgb phong_shading_with_clamp(const Vector3f& eye_vector, const Vector3f& light_vector, const Vector3f& normal_vector, ImageProcessing::Color_rgb material_color)
{
    float ambient_intensity = 10.0f;
    float specular_power = 15.0f;

    const float diffuse_intensity = std::max(dot(light_vector, normal_vector), 0.0f);
    float specular_intensity = 0.0f;
    if(diffuse_intensity > 0.0f)
    {
        const Vector3f reflection_vector = 2.0f * diffuse_intensity * normal_vector - light_vector;
        const float specular = std::max(dot(reflection_vector, eye_vector), 0.0f);
        specular_intensity = powf(specular, specular_power);
    }

    ImageProcessing::Color_rgb color;
    color.red   = static_cast<unsigned char>(std::min(ambient_intensity + material_color.red   * diffuse_intensity + 255.0f * specular_intensity, 255.0f));
    color.green = static_cast<unsigned char>(std::min(ambient_intensity + material_color.green * diffuse_intensity + 255.0f * specular_intensity, 255.0f));
    color.blue  = static_cast<unsigned char>(std::min(ambient_intensity + material_color.blue  * diffuse_intensity + 255.0f * specular_intensity, 255.0f));

    return color;
}

ImageProcessing::Bitmap get_ray_traced_bitmap()
{
    int width = 512;
    int height = 512;
    float near_plane = -1.0f;

    const std::vector<Sphere> objects =
    {
        { {0.0f,   0.0f,  -2.0f}, 0.5f },
        { {10.0f, 10.0f, -20.0f}, 5.5f }
    };

    constexpr Vector3f light_source = {-1.0f, 1.0f, 0.0f};
    constexpr Vector3f eye_origin = {0.0f, 0.0f, 0.0f};
    //constexpr Vector3f look_at = {0.0f, 0.0f, -1.0f};

    const ImageProcessing::Color_rgb material_color = {255, 0, 0};

    ImageProcessing::Bitmap bitmap;
    bitmap.height = height;
    bitmap.width = width;
    bitmap.filtered = true;
    bitmap.bitmap.resize(height * width * 3);

    constexpr int samples_per_direction = 2;
    constexpr float sample_filter = 1.0f / (samples_per_direction * samples_per_direction);

    for(int j = 0; j < height; ++j)
    {
        for(int i = 0; i < width; ++i)
        {
            ImageProcessing::Color_rgb final_color = {0, 0, 0};

            // Average multiple rays per pixel.
            for(int s = 0; s < samples_per_direction; ++s)
            {
                for(int t = 0; t < samples_per_direction; ++t)
                {
                    // TODO: 2016: ray direction needs to use camera.
                    // ray_direction calculation assumes ray_origin is {0,0,0}.
                    const Vector3f ray_direction = normalize({static_cast<float>(i * samples_per_direction + t) / (width * samples_per_direction - 1) * 2 - 1,
                                                              ((height * samples_per_direction - 1) - static_cast<float>(j * samples_per_direction + s)) / (height * samples_per_direction - 1) * 2 - 1,
                                                              near_plane});

                    Vector3f intersection_point;
                    const Sphere* sphere = find_nearest_intersection(eye_origin, ray_direction, objects, &intersection_point);

                    if(sphere != nullptr)
                    {
                        Vector3f normal = normalize(intersection_point - sphere->center);
                        Vector3f light = normalize(light_source - intersection_point);

                        ImageProcessing::Color_rgb color = phong_shading_with_clamp(normalize(eye_origin - intersection_point), light, normal, material_color);
                        final_color.red += static_cast<uint8_t>(color.red * sample_filter);
                        final_color.green += static_cast<uint8_t>(color.green * sample_filter);
                        final_color.blue += static_cast<uint8_t>(color.blue * sample_filter);
                    }
                }
            }

            bitmap.bitmap[j * width * 3 + i * 3 + 0] = final_color.red;
            bitmap.bitmap[j * width * 3 + i * 3 + 1] = final_color.green;
            bitmap.bitmap[j * width * 3 + i * 3 + 2] = final_color.blue;
        }
    }

    const auto filter = ImageProcessing::generate_simple_box_filter(3);
    bitmap = ImageProcessing::apply_box_filter(filter, 3, bitmap);

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
#if 0
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

