#ifndef UNICODE_H
#define UNICODE_H

namespace Encoding
{

std::wstring utf16_from_utf8(const std::string& utf8_string);
//bool utf8_is_ascii(_In_z_ const char* utf8_string);

}

#endif

