#ifndef BITMAP_H
#define BITMAP_H

struct Bitmap
{
    Bitmap();

// http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
#if _MSC_VER == 1600 || _MSC_VER == 1700
    // Add default and move constructors as VS2010/2012 doesn't default define a move constructor.
    Bitmap(Bitmap&& other);
    Bitmap& operator=(Bitmap&& other) NOEXCEPT;
#else
#error This compiler may autodefine the default move constructor.
#endif

    std::unique_ptr<uint8_t[]> bitmap;
    int xsize;
    int ysize;
    bool filtered;
};

Bitmap bitmap_from_file(_In_ const char* file_name);

#endif

