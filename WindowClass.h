#ifndef WINDOWCLASS_H
#define WINDOWCLASS_H

#include "PreCPP11.h"
#include "ScopedWindowsTypes.h"

namespace WindowsCommon
{

WNDCLASSEX get_default_blank_window_class(_In_ HINSTANCE instance, _In_ WNDPROC window_proc, _In_ PCTSTR window_class_name) NOEXCEPT;
Scoped_atom register_window_class(const WNDCLASSEX& window_class);

}

#endif

