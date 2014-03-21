#include "PreCompile.h"
#include "WindowClass.h"    // Pick up forward declarations to ensure correctness.
#include "HRException.h"

namespace WindowsCommon
{

WNDCLASSEX get_default_blank_window_class(_In_ HINSTANCE instance, _In_ WNDPROC window_proc, _In_ PCTSTR window_class_name) NOEXCEPT
{
    WNDCLASSEX window_class;
    window_class.cbSize        = sizeof(window_class);
    window_class.style         = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = window_proc;
    window_class.cbClsExtra    = 0;
    window_class.cbWndExtra    = 0;
    window_class.hInstance     = instance;
    window_class.hIcon         = nullptr;
    window_class.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    window_class.hbrBackground = nullptr;
    window_class.lpszMenuName  = nullptr;
    window_class.lpszClassName = window_class_name;
    window_class.hIconSm       = window_class.hIcon;

    // Return value optimization expected.
    return window_class;
}

Scoped_atom register_window_class(const WNDCLASSEX& window_class, const std::function<void (ATOM)>& unregister_class)
{
    Scoped_atom atom(RegisterClassEx(&window_class), unregister_class);

    if(0 == atom)
    {
        HRESULT hr = WindowsCommon::hresult_from_last_error();
        assert(HRESULT_FROM_WIN32(ERROR_CLASS_ALREADY_EXISTS) != hr);
        WindowsCommon::throw_hr(hr);
    }

    return std::move(atom);
}

static void unregister_atom(_In_ ATOM atom, _In_ HINSTANCE instance) NOEXCEPT
{
    BOOL result = UnregisterClass(MAKEINTATOM(atom), instance);
    if(!result)
    {
        auto hr = WindowsCommon::hresult_from_last_error();
        (hr);
        assert(SUCCEEDED(hr));
    }
}

std::function<void (ATOM)> unregister_class_functor(_In_ HINSTANCE instance)
{
    std::function<void (ATOM)> unregister_class(std::bind(WindowsCommon::unregister_atom, std::placeholders::_1, instance));
    return unregister_class;
}

}

