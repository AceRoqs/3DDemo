#include "PreCompile.h"

namespace Encoding
{

// TODO: Return code_point/index pair instead of a reference.
static uint32_t code_point_from_utf8_string(const std::string& utf8_string, size_t& index)
{
    const size_t length = utf8_string.length();

    uint32_t code_point;

    // Convert UTF-8 character to code point.

    // U+0000 - U+007F.  One byte.
    int continuation_byte_count = 0;
    if((utf8_string[index] & 0x80) == 0)
    {
        code_point = utf8_string[index];
    }
    // U+0080 - U+07FF.  Two bytes.
    else if((utf8_string[index] & 0xe0) == 0xc0)
    {
        code_point = utf8_string[index] & 0x1f;
        continuation_byte_count = 1;
    }
    // U+0800 - U+FFFF.  Three bytes.
    else if((utf8_string[index] & 0xf0) == 0xe0)
    {
        code_point = utf8_string[index] & 0x0f;
        continuation_byte_count = 2;
    }
    // U+10000 - U+1FFFFF.  Four bytes.
    else if((utf8_string[index] & 0xf8) == 0xf0)
    {
        code_point = utf8_string[index] & 7;
        continuation_byte_count = 3;
    }
    else
    {
        throw std::exception();
    }

    if(index + continuation_byte_count > length)
    {
        throw std::exception();
    }

    while(continuation_byte_count > 0)
    {
        ++index;
        code_point <<= 6;
        code_point += utf8_string[index] & 0x3f;
        --continuation_byte_count;

        // Continuation bytes must have the top two bits set to 10.
        if((utf8_string[index] & 0xc0) != 0x80)
        {
            throw std::exception();
        }
    }

    return code_point;
}

static wchar_t utf16_from_code_point(uint32_t code_point)
{
    // UTF-8 that encodes U+D800 to U+DFFF is an error per spec.
    if(code_point >= 0xd800 && code_point <= 0xdfff)
    {
        throw std::exception();
    }

    return static_cast<wchar_t>(code_point);
}

static wchar_t lead_surrogate_from_code_point(uint32_t code_point)
{
    code_point -= 0x010000;
    assert(code_point < 0xfffff);

    const uint16_t lead_surrogate = ((code_point >> 10) & 0x3ff) + 0xd800;
    return lead_surrogate;
}

static wchar_t trail_surrogate_from_code_point(uint32_t code_point)
{
    code_point -= 0x010000;
    assert(code_point < 0xfffff);

    const uint16_t trail_surrogate = (code_point & 0x3ff) + 0xdc00;
    return trail_surrogate;
}

// TODO: Find a way to do this without passing a mutable reference.  The challenge is that
// either one or two characters can be appended, depending on the passed value.
// Dependent types might be nice...
static void encode_and_append_code_point(std::wstring& utf16_string, uint32_t code_point)
{
    // Convert code point to UTF-16 character.

    // Basic Multilingual Plane - U+0000 to U+D7FF and U+E000 to U+FFFF.
    if(code_point < 0x10000)
    {
        utf16_string += utf16_from_code_point(code_point);
    }
    // Supplementary Planes.
    else
    {
        utf16_string += lead_surrogate_from_code_point(code_point);
        utf16_string += trail_surrogate_from_code_point(code_point);;
    }
}

// Does not write a BOM at the beginning of the returned string, as wchar_t is
// endian neutral until serialization.
std::wstring utf16_from_utf8(const std::string& utf8_string)
{
    std::wstring utf16_string;

    const size_t length = utf8_string.length();
    for(size_t ix = 0; ix < length; ++ix)
    {
        uint32_t code_point = code_point_from_utf8_string(utf8_string, ix);
        encode_and_append_code_point(utf16_string, code_point);
    }

    return utf16_string;
}

// Code point   UTF-16      UTF-8
// U+007A       007A        7A
// TODO: Add 2-byte UTF-8 sequence.
// U+6C34       6C34        E6 B0 B4
// U+10000      D800 DC00   F0 90 80 80
// U+1D11E      D834 DD1E   F0 9D 84 9E
// U+10FFFD     DBFF DFFD   F4 8F BF BD

static char utf8_case1[] = "\x7a";
static char utf8_case2[] = "\xe6\xb0\xb4";
static char utf8_case3[] = "\xf0\x90\x80\x80";
static char utf8_case4[] = "\xf0\x9d\x84\x9e";
static char utf8_case5[] = "\xf4\x8f\xbf\xbd";

// TODO: Enforce encoding of these as little endian.
static wchar_t utf16_case1[] = L"\x7a";
static wchar_t utf16_case2[] = L"\x6c34";
static wchar_t utf16_case3[] = L"\xd800\xdc00";
static wchar_t utf16_case4[] = L"\xd834\xdd1e";
static wchar_t utf16_case5[] = L"\xdbff\xdffd";

static bool test_case1()
{
    auto utf16_string = utf16le_from_utf8(utf8_case1);
    return utf16_string.compare(utf16_case1) == 0;
}

static bool test_case2()
{
    auto utf16_string = utf16le_from_utf8(utf8_case2);
    return utf16_string.compare(utf16_case2) == 0;
}

static bool test_case3()
{
    auto utf16_string = utf16le_from_utf8(utf8_case3);
    return utf16_string.compare(utf16_case3) == 0;
}

static bool test_case4()
{
    auto utf16_string = utf16le_from_utf8(utf8_case4);
    return utf16_string.compare(utf16_case4) == 0;
}

static bool test_case5()
{
    auto utf16_string = utf16le_from_utf8(utf8_case5);
    return utf16_string.compare(utf16_case5) == 0;
}

void test()
{
#ifndef NDEBUG
    const bool case1 = test_case1();
    const bool case2 = test_case2();
    const bool case3 = test_case3();
    const bool case4 = test_case4();
    const bool case5 = test_case5();

    assert(case1 && case2 && case3 && case4 && case5);
#endif
}

}

