#include "PreCompile.h"
#include "Tracing.h"

namespace WindowsCommon
{

void dprintf(_In_ PCSTR format, ...) NOEXCEPT
{
#ifndef NDEBUG
    va_list args;
    va_start(args, format);

    char print_buffer[1024];
    StringCbVPrintfA(print_buffer, sizeof(print_buffer), format, args);

    va_end(args);

    OutputDebugStringA(print_buffer);
#else
    UNREFERENCED_PARAMETER(format);
#endif
}

}

