#ifndef SCOPEDWINDOWSTYPES_H
#define SCOPEDWINDOWSTYPES_H

namespace WindowsCommon
{

typedef std_opt::unique_resource_t<ATOM, std::function<void (ATOM)>> Scoped_atom;
typedef std_opt::unique_resource_t<HWND, std::function<void (HWND)>> Scoped_window;

Scoped_atom make_scoped_window_class(_In_ ATOM atom, _In_ HINSTANCE instance);
Scoped_window make_scoped_window(_In_ HWND window);

}

#endif

