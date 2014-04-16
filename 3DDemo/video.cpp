//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: Video initialization routines for windows
//=========================================================================
#include "PreCompile.h"
#include "video.h"
#include "HRException.h"
#include "WindowClass.h"
#include "WindowsGL.h"

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
WindowsCommon::WGL_state Startup_OpenGL(_In_ HINSTANCE instance, bool fWindowed, WindowsCommon::Window_procedure* window_proc)
{
    const int window_width = 800;
    const int window_height = 600;

    PCTSTR app_title = TEXT("3D Demo 1999 (Updated for C++11)");

    const WNDCLASSEX window_class = WindowsCommon::get_default_blank_window_class(instance, WindowsCommon::Window_procedure::static_window_proc, app_title);

    WindowsCommon::WGL_state state;
    state.atom = WindowsCommon::register_window_class(window_class);

    if(fWindowed)
    {
        state.window = WindowsCommon::create_normal_window(app_title, app_title, window_width, window_height, instance, window_proc);
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

    if(!is_window_32bits_per_pixel(state.window))
    {
        // TODO: 2014: This message text is good - find some way to pass this via the exception.
        //MessageBox(window, TEXT("3D Engine demo requires 32-bit color."), TEXT("System requirements"), MB_OK);
        WindowsCommon::throw_hr(E_FAIL);
    }

    state.device_context = get_device_context(state.window);
    state.gl_context = create_gl_context(state.device_context);
    state.make_current_context = create_current_context(state.device_context, state.gl_context);

    return state;
}

void Shutdown_OpenGL(bool fWindowed)
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

