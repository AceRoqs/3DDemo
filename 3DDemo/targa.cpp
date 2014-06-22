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

static enum TGA_alpha_type
{
    No_alpha = 0,                       // No alpha data exists.
    Ignorable_alpha = 1,                // Alpha data can be ignored.
    Retained_alpha = 2,                 // Alpha data is undefined, but should be retained.
    Alpha_exists = 3,                   // Alpha data exists.
    Premultiplied_alpha = 4,            // Alpha is pre-multiplied.
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
    uint32_t extension_area_offset;         // Offset in bytes from the beginning of the file.
    uint32_t developer_directory_offset;    // Offset in bytes from the beginning of the file.
    char     signature[18];                 // "TRUEVISION-XFILE.".
};

struct TGA_developer_directory_entry
{
    uint16_t tag;                       // Application specific tag ID.
    uint32_t developer_field_offset;    // Offset in bytes from the beginning of the file.
    uint32_t developer_field_size;      // Size in bytes.
};

struct TGA_developer_directory
{
    uint16_t entry_count;
    TGA_developer_directory_entry entries[1];   // entry_count number of entries.
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
    uint32_t color_correction_offset;   // Offset in bytes from the beginning of the file.
    uint32_t thumbnail_image_offset;    // Offset in bytes from the beginning of the file.
    uint32_t scanline_table_offset;     // Offset in bytes from the beginning of the file
    uint8_t  attributes_type;           // TGA_alpha_type.
} ;

#pragma pack(pop)

void validate_tga_header(_In_ const TGA_header* header)
{
    bool succeeded = true;

    succeeded &= (header->image_type == True_color);
    succeeded &= (header->bits_per_pixel == 24);
    succeeded &= (header->color_map_length == 0);
    succeeded &= (header->color_map_bits_per_pixel == 0);

    // Bound the size as this is used in buffer size calculations, which may need to fit in 
    succeeded &= (header->image_width <= 16384);
    succeeded &= (header->image_height <= 16384);

    // id_length is unbounded.

    if(!succeeded)
    {
        throw std::exception();
    }
}

size_t get_pixel_data_offset(_In_ const TGA_header* header)
{
    return sizeof(TGA_header) +
           header->id_length +
           header->color_map_length * (header->color_map_bits_per_pixel / 8);
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

    const size_t pixel_count = header->image_width * header->image_height * (header->bits_per_pixel / 8) / sizeof(Color_rgb);
    bitmap.bitmap.resize(pixel_count);

    // TODO: prevent read overrun past size.

    // MSVC complains that std::copy is insecure.
    //std::copy(reinterpret_cast<const Color_rgb*>(&file[pixel_data_offset]), reinterpret_cast<const Color_rgb*>(&file[pixel_data_offset]) + pixel_count, &bitmap.bitmap[0]);
    const size_t pixel_data_offset = get_pixel_data_offset(header);
    memcpy(&bitmap.bitmap[0], tga_memory + pixel_data_offset, pixel_count * sizeof(Color_rgb));

    return bitmap;
}

