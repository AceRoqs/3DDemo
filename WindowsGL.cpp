#include "PreCompile.h"
#include "WindowsGL.h"      // Pick up forward declarations to ensure correctness.
#include "HRException.h"
#include "WindowClass.h"

namespace WindowsCommon
{

static bool is_window_32bits_per_pixel(_In_ HWND window)
{
    WindowsCommon::Scoped_device_context device_context = WindowsCommon::get_device_context(window);

    if(::GetDeviceCaps(device_context, BITSPIXEL) < 32)
    {
        return false;
    }

    return true;
}

// TODO: set window width/height if full screen
WindowGL_window_procedure::WindowGL_window_procedure(_In_ HINSTANCE instance, bool windowed) : m_windowed(windowed)
{
    const int window_width = 800;
    const int window_height = 600;

    PCTSTR app_title = TEXT("3D Demo 1999 (Updated for C++11)");

    const WNDCLASSEX window_class = WindowsCommon::get_default_blank_window_class(instance, WindowsCommon::Window_procedure::static_window_proc, app_title);

    m_state.atom = WindowsCommon::register_window_class(window_class);

    if(windowed)
    {
        m_state.window = WindowsCommon::create_normal_window(app_title, app_title, window_width, window_height, instance, this);
    }
    else
    {
        DEVMODE DevMode;
        ZeroMemory(&DevMode, sizeof(DEVMODE));
        DevMode.dmSize = sizeof(DEVMODE);
        DevMode.dmBitsPerPel = 32;
        DevMode.dmPelsWidth = 640;
        DevMode.dmPelsHeight = 480;
        DevMode.dmFields = DM_BITSPERPEL;

        ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
        DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
        ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);

        m_state.window = WindowsCommon::create_window(
            app_title,
            app_title,
            WS_POPUP | WS_CLIPSIBLINGS,
            0,
            0,
            DevMode.dmPelsWidth,
            DevMode.dmPelsHeight,
            nullptr,
            nullptr,
            instance,
            nullptr);

        ShowCursor(false);
    }

    if(!is_window_32bits_per_pixel(m_state.window))
    {
        // TODO: 2014: This message text is good - find some way to pass this via the exception.
        //MessageBox(window, TEXT("3D Engine demo requires 32-bit color."), TEXT("System requirements"), MB_OK);
        WindowsCommon::throw_hr(E_FAIL);
    }

    m_state.device_context = get_device_context(m_state.window);
    m_state.gl_context = create_gl_context(m_state.device_context);
    m_state.make_current_context = create_current_context(m_state.device_context, m_state.gl_context);
}

WindowGL_window_procedure::~WindowGL_window_procedure()
{
    // TODO11: This is releasing a DC and destroying a window that
    // has already finished the message loop.  i.e. The window is
    // already destroyed.  Handle this stuff in WM_CLOSE.
    // Does this mean that WM_CREATE should be used for init??

    // TODO: only shutdown if valid GL context
    // release OpenGL resource context
    //::wglMakeCurrent(nullptr, nullptr);
    //::wglDeleteContext(state.gl_context);
    //::ReleaseDC(hwnd, hdc);
    if(!m_windowed)
    {
        ::ChangeDisplaySettings(nullptr, 0);
    }

    //::DestroyWindow(hwnd);
}

LRESULT WindowGL_window_procedure::window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT return_value = 0;

    switch(message)
    {
        case WM_SIZE:
        {
            RECT client_rectangle;
            ::GetClientRect(window, &client_rectangle);

            ::glViewport(client_rectangle.left, client_rectangle.top, client_rectangle.right, client_rectangle.bottom);

            break;
        }

        case WM_ERASEBKGND:
        {
            break;
        }

        default:
        {
            return_value = DefWindowProc(window, message, w_param, l_param);
            break;
        }
    }

    return return_value;
}

}

