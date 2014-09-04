#include "PreCompile.h"
#include "Unicode.h"        // Pick up forward declarations to ensure correctness.
#include "CheckException.h"

namespace PortableRuntime
{

struct UTF8_descriptor
{
    uint32_t code_point;
    unsigned int sequence_length;
};

static UTF8_descriptor descriptor_from_utf8_index(const std::string& utf8_string, size_t index)
{
    const size_t length = utf8_string.length();

    UTF8_descriptor descriptor;

    // Convert UTF-8 character to code point.

    check_exception(((utf8_string[index] & 0x80) == 0) ||
                    ((utf8_string[index] & 0xe0) == 0xc0) ||
                    ((utf8_string[index] & 0xf0) == 0xe0) ||
                    ((utf8_string[index] & 0xf8) == 0xf0));

    // U+0000 - U+007F.  One byte.
    if((utf8_string[index] & 0x80) == 0)
    {
        descriptor.code_point = utf8_string[index];
        descriptor.sequence_length = 1;
    }
    // U+0080 - U+07FF.  Two bytes.
    else if((utf8_string[index] & 0xe0) == 0xc0)
    {
        descriptor.code_point = utf8_string[index] & 0x1f;
        descriptor.sequence_length = 2;
    }
    // U+0800 - U+FFFF.  Three bytes.
    else if((utf8_string[index] & 0xf0) == 0xe0)
    {
        descriptor.code_point = utf8_string[index] & 0x0f;
        descriptor.sequence_length = 3;
    }
    // U+10000 - U+1FFFFF.  Four bytes.
    else // utf8_string[index] & 0xf8) == 0xf0.
    {
        descriptor.code_point = utf8_string[index] & 7;
        descriptor.sequence_length = 4;
    }

    check_exception(index + descriptor.sequence_length <= length);

    std::for_each(&utf8_string[index + 1], &utf8_string[index + descriptor.sequence_length], [&descriptor](char ch)
    {
        descriptor.code_point <<= 6;
        descriptor.code_point += ch & 0x3f;

        // Continuation bytes must have the top two bits set to 10.
        check_exception((ch & 0xc0) == 0x80);
    });

    return descriptor;
}

static wchar_t utf16_from_bmp_code_point(uint32_t code_point)
{
    // UTF-8 that encodes U+D800 to U+DFFF is an error per spec.
    check_exception((code_point < 0xd800) || (code_point > 0xdfff));

    return static_cast<wchar_t>(code_point);
}

static wchar_t lead_surrogate_from_code_point(uint32_t code_point) NOEXCEPT
{
    code_point -= 0x010000;
    assert(code_point < 0xfffff);

    const uint16_t lead_surrogate = ((code_point >> 10) & 0x3ff) + 0xd800;
    return lead_surrogate;
}

static wchar_t trail_surrogate_from_code_point(uint32_t code_point) NOEXCEPT
{
    code_point -= 0x010000;
    assert(code_point < 0xfffff);

    const uint16_t trail_surrogate = (code_point & 0x3ff) + 0xdc00;
    return trail_surrogate;
}

static std::wstring utf16_encode_code_point(uint32_t code_point)
{
    // Convert code point to UTF-16 character.
    std::wstring utf16_string;

    // Basic Multilingual Plane - U+0000 to U+D7FF and U+E000 to U+FFFF.
    if(code_point < 0x10000)
    {
        utf16_string += utf16_from_bmp_code_point(code_point);
    }
    // Supplementary Planes.
    else
    {
        utf16_string += lead_surrogate_from_code_point(code_point);
        utf16_string += trail_surrogate_from_code_point(code_point);
    }

    return utf16_string;
}

// Does not write a BOM at the beginning of the returned string, as wchar_t is
// endian neutral until serialization.
std::wstring utf16_from_utf8(const std::string& utf8_string)
{
    std::wstring utf16_string;

    const size_t length = utf8_string.length();
    for(size_t index = 0; index < length; )
    {
        // TODO: Single append for each character is too expensive.
        const auto descriptor = descriptor_from_utf8_index(utf8_string, index);
        utf16_string += utf16_encode_code_point(descriptor.code_point);
        index += descriptor.sequence_length;
    }

    return utf16_string;
}

// Untested, and not used anywhere.
#if 0
bool utf8_is_ascii(_In_z_ const char* utf8_string)
{
    bool is_ascii = true;

    while(*utf8_string != '\0')
    {
        if(*utf8_string < 0)
        {
            is_ascii = false;
            break;
        }
        ++utf8_string;
    }

    return is_ascii;
}
#endif

// Code point   UTF-16      UTF-8
// U+007A       007A        7A
// U+00E9       00E9        C3 A9
// U+6C34       6C34        E6 B0 B4
// U+10000      D800 DC00   F0 90 80 80
// U+1D11E      D834 DD1E   F0 9D 84 9E
// U+10FFFD     DBFF DFFD   F4 8F BF BD

static char utf8_case1[] = "\x7a";
static char utf8_case2[] = "\xc3\xa9";
static char utf8_case3[] = "\xe6\xb0\xb4";
static char utf8_case4[] = "\xf0\x90\x80\x80";
static char utf8_case5[] = "\xf0\x9d\x84\x9e";
static char utf8_case6[] = "\xf4\x8f\xbf\xbd";

// TODO: Enforce encoding of these as little endian.
static wchar_t utf16_case1[] = L"\x7a";
static wchar_t utf16_case2[] = L"\x00e9";
static wchar_t utf16_case3[] = L"\x6c34";
static wchar_t utf16_case4[] = L"\xd800\xdc00";
static wchar_t utf16_case5[] = L"\xd834\xdd1e";
static wchar_t utf16_case6[] = L"\xdbff\xdffd";

static bool test_case1()
{
    auto utf16_string = utf16_from_utf8(utf8_case1);
    return utf16_string.compare(utf16_case1) == 0;
}

static bool test_case2()
{
    auto utf16_string = utf16_from_utf8(utf8_case2);
    return utf16_string.compare(utf16_case2) == 0;
}

static bool test_case3()
{
    auto utf16_string = utf16_from_utf8(utf8_case3);
    return utf16_string.compare(utf16_case3) == 0;
}

static bool test_case4()
{
    auto utf16_string = utf16_from_utf8(utf8_case4);
    return utf16_string.compare(utf16_case4) == 0;
}

static bool test_case5()
{
    auto utf16_string = utf16_from_utf8(utf8_case5);
    return utf16_string.compare(utf16_case5) == 0;
}

static bool test_case6()
{
    auto utf16_string = utf16_from_utf8(utf8_case6);
    return utf16_string.compare(utf16_case6) == 0;
}

void test()
{
#ifndef NDEBUG
    const bool case1 = test_case1();
    const bool case2 = test_case2();
    const bool case3 = test_case3();
    const bool case4 = test_case4();
    const bool case5 = test_case5();
    const bool case6 = test_case6();

    assert(case1 && case2 && case3 && case4 && case5 && case6);
#endif
}

}

