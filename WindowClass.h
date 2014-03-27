#ifndef WINDOWCLASS_H
#define WINDOWCLASS_H

#include "PreCPP11.h"
#include "ScopedWindowsTypes.h"

namespace WindowsCommon
{

WNDCLASSEX get_default_blank_window_class(_In_ HINSTANCE instance, _In_ WNDPROC window_proc, _In_ PCTSTR window_class_name) NOEXCEPT;
Scoped_atom register_window_class(const WNDCLASSEX& window_class);

Scoped_window create_window(_In_opt_ PCWSTR class_name, _In_opt_ PCWSTR window_name, DWORD style, int x, int y,
    int width, int height, _In_opt_ HWND parent, _In_opt_ HMENU menu, _In_opt_ HINSTANCE instance, _In_opt_ PVOID param);
Scoped_window create_normal_window(_In_ PCTSTR window_class_name, _In_ PCTSTR title, int width, int height, _In_opt_ HINSTANCE instance, _In_opt_ PVOID param);

}

#endif

