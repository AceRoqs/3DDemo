//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: Video initialization routines for windows
//=========================================================================
#include "PreCompile.h"
#include "video.h"
#include "HRException.h"
#include "resource.h"
#include "WindowClass.h"
#include "WGLState.h"

static const int window_width = 800;
static const int window_height = 600;

static WindowsCommon::WGL_state Startup_OpenGL(HINSTANCE hInstance, bool fWindowed);
static void Shutdown_OpenGL(bool fWindowed);

//---------------------------------------------------------------------------
bool is_window_32bits_per_pixel(_In_ HWND window)
{
    WindowsCommon::Scoped_device_context device_context = WindowsCommon::get_device_context(window);

    if(::GetDeviceCaps(device_context, BITSPIXEL) < 32)
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
LRESULT CALLBACK window_proc(
    HWND window,
    UINT message,
    WPARAM w_param,
    LPARAM l_param)
{
    LRESULT return_value = 0;

    switch(message)
    {
        case WM_CREATE:
        {
            if(!is_window_32bits_per_pixel(window))
            {
                ::MessageBox(window,
                             TEXT("3D Engine demo requires 32-bit color."),
                             TEXT("System requirements"),
                             MB_OK);
                ::DestroyWindow(window);
            }

            break;
        }

        case WM_SIZE:
        {
            RECT rect;
            ::GetClientRect(window, &rect);

            // TODO: remove OpenGL dependancy
            ::glViewport(rect.left, rect.top, rect.right, rect.bottom);

            break;
        }

        case WM_ACTIVATEAPP:
        {
            if(static_cast<BOOL>(w_param))
            {
                // NOTE: Reload any changed resources here.
            }

            break;
        }

#if 0
        case WM_SYSKEYDOWN:
        {
            if(VK_RETURN == wParam)
            {
//                MessageBox(window, "Toggle full screen mode here", "NOTIMPLEMENTED", 0);
//                Shutdown_Video(s_fWindowed);

                EndEngine();
                s_fWindowed = !s_fWindowed;
//                Startup_Video(GetModuleHandle(nullptr), SW_SHOW, s_fWindowed);
//                StartLoad();
                InitEngine(GetModuleHandle(nullptr), SW_SHOW);
                // TODO: this leaks the dynamic arrays?
                start_load("polydefs.txt");
            }
            else if(VK_F4 == wParam)
            {
                DestroyWindow(window);
            }

            break;
        }
#endif

        case WM_ERASEBKGND:
        {
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        default:
        {
            return_value = DefWindowProc(window, message, w_param, l_param);
            break;
        }
    }

    return return_value;
}

//---------------------------------------------------------------------------
// Startup_Video()
//---------------------------------------------------------------------------
WindowsCommon::WGL_state Startup_Video(_In_ HINSTANCE hInstance, bool fWindowed)
{
    return Startup_OpenGL(hInstance, fWindowed);
}

//---------------------------------------------------------------------------
// Shutdown_Video()
//---------------------------------------------------------------------------
void Shutdown_Video(
    bool fWindowed)
{
    Shutdown_OpenGL(fWindowed);
}

//---------------------------------------------------------------------------
// Startup_OpenGL()
//---------------------------------------------------------------------------
// TODO: set window width/height if full screen
static WindowsCommon::WGL_state Startup_OpenGL(_In_ HINSTANCE instance, bool fWindowed)
{
    PCTSTR app_title = TEXT("3D Demo 1999 (Updated for C++11)");

    const WNDCLASSEX window_class = WindowsCommon::get_default_blank_window_class(instance, window_proc, app_title);

    WindowsCommon::WGL_state state;
    state.atom = WindowsCommon::register_window_class(window_class);

    if(fWindowed)
    {
        state.window = WindowsCommon::create_normal_window(app_title, app_title, window_width, window_height, instance, nullptr);
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

        state.window = WindowsCommon::create_window(
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

    state.device_context = get_device_context(state.window);
    state.gl_context = create_gl_context(state.device_context);
    state.make_current_context = create_current_context(state.device_context, state.gl_context);

    return state;
}

//---------------------------------------------------------------------------
static void Shutdown_OpenGL(
    bool fWindowed)
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
    if(!fWindowed)
    {
        ::ChangeDisplaySettings(nullptr, 0);
    }

    //::DestroyWindow(hwnd);
}

