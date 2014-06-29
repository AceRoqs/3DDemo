#ifndef WINDOWCLASS_H
#define WINDOWCLASS_H

#include "ScopedWindowsTypes.h"

namespace WindowsCommon
{

class Window_procedure
{
public:
    static LRESULT CALLBACK static_window_proc(__in HWND window, UINT message, WPARAM w_param, LPARAM l_param) NOEXCEPT;

protected:
    virtual LRESULT window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param) NOEXCEPT = 0;
};

bool dispatch_all_windows_messages(_Out_ MSG* message) NOEXCEPT;

WNDCLASSEX get_default_blank_window_class(_In_ HINSTANCE instance, _In_ WNDPROC window_proc, _In_ PCTSTR window_class_name) NOEXCEPT;
Scoped_atom register_window_class(const WNDCLASSEX& window_class);

Scoped_window create_window(_In_opt_ PCTSTR class_name, _In_opt_ PCTSTR window_name, DWORD style, int x, int y,
    int width, int height, _In_opt_ HWND parent, _In_opt_ HMENU menu, _In_opt_ HINSTANCE instance, _In_opt_ PVOID param);
Scoped_window create_normal_window(_In_ PCTSTR window_class_name, _In_ PCTSTR title, int width, int height, _In_opt_ HINSTANCE instance, _In_opt_ PVOID param);
Scoped_device_context get_device_context(_In_ HWND window);
Scoped_gl_context create_gl_context(_In_ HDC device_context);
Scoped_current_context create_current_context(_In_ HDC device_context, _In_ HGLRC gl_context);
Scoped_handle create_file(_In_ PCTSTR file_name, DWORD desired_access, DWORD share_mode,
    _In_opt_ PSECURITY_ATTRIBUTES security_attributes, DWORD creation_disposition, DWORD flags, _In_opt_ HANDLE template_file);

}

#endif

