#ifndef BLOCK_H
#define BLOCK_H

// TODO: Move implementations out of header.
struct block_t
{
    block_t() : xsize(0), ysize(0), filtered(false) {}
// http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
#if _MSC_VER == 1600 || _MSC_VER == 1700
    // Add default and move constructors as VS2010/2012 doesn't default define a move constructor.
    block_t(block_t&& other) :
        bitmap(std::move(other.bitmap)),
        xsize(other.xsize),
        ysize(other.ysize),
        filtered(other.filtered)
    {
    }

    block_t& operator=(block_t&& other) NOEXCEPT
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

    std::unique_ptr<uint8_t[]> bitmap;
    int xsize;
    int ysize;
    bool filtered;
};

#endif

