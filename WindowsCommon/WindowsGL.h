#ifndef WGLSTATE_H
#define WGLSTATE_H

#include "ScopedWindowsTypes.h"
#include "WindowClass.h"

namespace WindowsCommon
{

struct WGL_state
{
// http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
#if _MSC_VER == 1600 || _MSC_VER == 1700
    // Add default and move constructors as VS2010/2012 doesn't default define a move constructor.
    WGL_state()
    {
    }

    WGL_state(WGL_state&& other) :
        atom(std::move(other.atom)),
        window(std::move(other.window)),
        device_context(std::move(other.device_context)),
        gl_context(std::move(other.gl_context)),
        make_current_context(std::move(other.make_current_context))
    {
    }

    WGL_state& operator=(WGL_state&& other) NOEXCEPT
    {
        // Handle A=A case.
        if(this != &other)
        {
            atom = std::move(other.atom);
            window = std::move(other.window);
            device_context = std::move(other.device_context);
            gl_context = std::move(other.gl_context);
            make_current_context = std::move(other.make_current_context);
        }

        return *this;
    }
#else
#error This compiler may autodefine the default move constructor.
#endif

    // The order of these fields matter, as destruction must happen in the opposite order.
    Scoped_atom atom;
    Scoped_window window;
    Scoped_device_context device_context;
    Scoped_gl_context gl_context;
    Scoped_current_context make_current_context;
};

class WindowGL_window_procedure : public Window_procedure
{
public:
    WindowGL_window_procedure(_In_ HINSTANCE instance, bool windowed);
    ~WindowGL_window_procedure();

    WindowsCommon::WGL_state m_state;

protected:
    LRESULT window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param);

private:
    bool m_windowed;
};

}

#endif

