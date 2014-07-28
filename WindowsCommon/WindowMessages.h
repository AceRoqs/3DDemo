#ifndef WINDOWMESSAGES_H
#define WINDOWMESSAGES_H

namespace WindowsCommon
{

void debug_validate_message_map() NOEXCEPT;
PCSTR string_from_window_message(UINT message) NOEXCEPT;

}

#endif

