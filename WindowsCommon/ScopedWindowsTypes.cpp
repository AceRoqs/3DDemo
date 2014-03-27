#include "PreCompile.h"
#include "ScopedWindowsTypes.h"     // Pick up forward declarations to ensure correctness.
#include "HRException.h"

namespace WindowsCommon
{

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

static std::function<void (ATOM)> unregister_class_functor(_In_ HINSTANCE instance)
{
    return [=](ATOM atom)
    {
        unregister_atom(atom, instance);
    };
}

Scoped_atom create_scoped_window_class(const WNDCLASSEX& window_class, _In_ HINSTANCE instance)
{
    ATOM atom = RegisterClassEx(&window_class);

    return std::move(std_opt::make_scoped_resource_checked(unregister_class_functor(instance), atom, static_cast<decltype(atom)>(0)));
}

static std::function<void (HWND)> destroy_window_functor()
{
    return [](_In_ HWND window)
    {
        if(!DestroyWindow(window))
        {
            auto hr = hresult_from_last_error();
            (hr);
            assert(SUCCEEDED(hr));
        }
    };
}

// TODO: create_scoped_window should take an HWND.  Otherwise this makes lazy evaluation difficult.
std_opt::scoped_resource<std::function<void (HWND)>, HWND> create_scoped_window(
    _In_opt_ PCWSTR class_name,
    _In_opt_ PCWSTR window_name,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    _In_opt_ HWND parent,
    _In_opt_ HMENU menu,
    _In_opt_ HINSTANCE instance,
    _In_opt_ PVOID param)
{
    HWND window = CreateWindow(class_name, window_name, style, x, y, width, height, parent, menu, instance, param);

    return std::move(std_opt::make_scoped_resource_checked(destroy_window_functor(), window, static_cast<decltype(window)>(nullptr)));
}

}

