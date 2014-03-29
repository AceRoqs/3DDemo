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

Scoped_atom make_scoped_window_class(_In_ ATOM atom, _In_ HINSTANCE instance)
{
    return std::move(std_opt::unique_resource_checked(atom, static_cast<decltype(atom)>(0), unregister_class_functor(instance)));
}

Scoped_atom2 make_scoped_window_class2(_In_ ATOM atom, _In_ HINSTANCE instance)
{
    return std::move(Scoped_atom2(atom, instance));
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

Scoped_window make_scoped_window(_In_ HWND window)
{
    // TODO: how can this take a function instead of a lambda for the deleter?
    return std::move(std_opt::unique_resource_checked(window, static_cast<decltype(window)>(nullptr), destroy_window_functor()));
}

}

