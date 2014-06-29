#include "PreCompile.h"

namespace Encoding
{

// Does not at a BOM at the beginning of the returned string.
std::wstring utf16le_from_utf8(const std::string& utf8_string)
{
    std::wstring utf16_string;

    const size_t length = utf8_string.length();
    for(size_t ix = 0; ix < length; ++ix)
    {
        uint32_t code_point;

        // Convert UTF-8 character to code point.

        // U+0000 - U+007F.  One byte.
        if(utf8_string[ix] < 128)
        {
            code_point = utf8_string[ix];
        }
        // U+0080 - U+07FF.  Two bytes.
        else if((utf8_string[ix] & 0xe0) == 0xc0)
        {
            code_point = utf8_string[ix] & 0x1f;
            code_point <<= 6;
            // TODO: Bounds check ix + 1;
            // TODO: check 10 binary start of trail bytes.
            code_point += utf8_string[ix + 1] & 0x3f;
            ++ix;
        }
        // U+0800 - U+FFFF.  Three bytes.
        else if((utf8_string[ix] & 0xf0) == 0xe0)
        {
            // TODO: Bounds check ix + 1, ix + 2;
            // TODO: check 10 binary start of trail bytes.
            code_point = utf8_string[ix] & 0x0f;
            code_point <<= 6;
            code_point += utf8_string[ix + 1] & 0x3f;
            code_point <<= 6;
            code_point += utf8_string[ix + 2] & 0x3f;
            ix += 2;
        }
        // U+10000 - U+1FFFFF.  Four bytes.
        else if((utf8_string[ix] & 0xf8) == 0xf0)
        {
            // TODO: Bounds check ix + 1, ix + 2, ix + 3;
            // TODO: check 10 binary start of trail bytes.
            code_point = utf8_string[ix] & 7;
            code_point <<= 6;
            code_point += utf8_string[ix + 1] & 0x3f;
            code_point <<= 6;
            code_point += utf8_string[ix + 2] & 0x3f;
            code_point <<= 6;
            code_point += utf8_string[ix + 3] & 0x3f;
            ix += 3;
        }
        else
        {
            throw std::exception();
        }

        // Convert code point to UTF-16LE character.

        // Basic Multilingual Plane - U+0000 to U+D7FF and U+E000 to U+FFFF.
        if(code_point < 0x10000)
        {
            // UTF-8 that encodes U+D800 to U+DFFF is an error per spec.
            if(code_point >= 0xd800 && code_point <= 0xdfff)
            {
                throw std::exception();
            }

            utf16_string += static_cast<wchar_t>(code_point);
        }
        else
        {
            code_point -= 0x010000;
            assert(code_point < 0xfffff);

            const wchar_t lead_surrogate = ((code_point >> 10) & 0x3ff) + 0xd800;
            const wchar_t trail_surrogate = (code_point & 0x3ff) + 0xdc00;

            utf16_string += lead_surrogate;
            utf16_string += trail_surrogate;
        }
    }

    return utf16_string;
}

}

