#ifndef SCOPEDWINDOWSTYPES_H
#define SCOPEDWINDOWSTYPES_H

namespace WindowsCommon
{

typedef std_opt::scoped_resource<std::function<void (ATOM)>, ATOM> Scoped_atom;
typedef std_opt::scoped_resource<std::function<void (HWND)>, HWND> Scoped_window;

Scoped_atom create_scoped_window_class(const WNDCLASSEX& window_class, _In_ HINSTANCE instance);
std_opt::scoped_resource<std::function<void (HWND)>, HWND> create_scoped_window(_In_opt_ PCWSTR class_name, _In_opt_ PCWSTR window_name,
    DWORD style, int x, int y, int width, int height, _In_opt_ HWND parent, _In_opt_ HMENU menu, _In_opt_ HINSTANCE instance, _In_opt_ PVOID param);

}

#endif

