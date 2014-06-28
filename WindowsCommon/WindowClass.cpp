#include "PreCompile.h"
#include "WindowClass.h"    // Pick up forward declarations to ensure correctness.
#include "HRException.h"

namespace WindowsCommon
{

LRESULT CALLBACK Window_procedure::static_window_proc(__in HWND window, UINT message, WPARAM w_param, LPARAM l_param) NOEXCEPT
{
    // Sent by CreateWindow.
    if(message == WM_NCCREATE)
    {
        CREATESTRUCT* create_struct = reinterpret_cast<CREATESTRUCT*>(l_param);

        // This function should never fail.
        const auto app = reinterpret_cast<Window_procedure*>(create_struct->lpCreateParams);
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }

    LRESULT return_value;

    // GetWindowLongPtr should never fail.
    // The variable 'app' is not valid until WM_NCCREATE has been sent.
    const auto app = reinterpret_cast<Window_procedure*>(GetWindowLongPtr(window, GWLP_USERDATA));
    if(app != nullptr)
    {
        return_value = app->window_proc(window, message, w_param, l_param);
    }
    else
    {
        return_value = DefWindowProc(window, message, w_param, l_param);
    }

    return return_value;
}

// Returns false if WM_QUIT was posted.
bool dispatch_all_windows_messages(_Out_ MSG* message) NOEXCEPT
{
    // Clear out all the messages before drawing a new frame.
    BOOL message_exists = PeekMessage(message, nullptr, 0, 0, PM_REMOVE);
    while(message_exists)
    {
        if(WM_QUIT == message->message)
        {
            break;
        }

        ::TranslateMessage(message);
        DispatchMessage(message);

        message_exists = PeekMessage(message, nullptr, 0, 0, PM_REMOVE);
    }

    return !message_exists || (WM_QUIT != message->message);
}

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

Scoped_atom register_window_class(const WNDCLASSEX& window_class)
{
    const auto atom = RegisterClassEx(&window_class);

    if(0 == atom)
    {
        HRESULT hr = hresult_from_last_error();
        assert(HRESULT_FROM_WIN32(ERROR_CLASS_ALREADY_EXISTS) != hr);
        WindowsCommon::throw_hr(hr);
    }

    return make_scoped_window_class(atom, window_class.hInstance);
}

// TODO: WindowClass.cpp is not the best place for this.
Scoped_window create_window(
    _In_opt_ PCTSTR class_name,
    _In_opt_ PCTSTR window_name,
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
    const auto window = CreateWindow(class_name, window_name, style, x, y, width, height, parent, menu, instance, param);

    if(nullptr == window)
    {
        HRESULT hr = hresult_from_last_error();
        WindowsCommon::throw_hr(hr);
    }

    return make_scoped_window(window);
}

Scoped_window create_normal_window(_In_ PCTSTR window_class_name, _In_ PCTSTR title, int width, int height, _In_opt_ HINSTANCE instance, _In_opt_ PVOID param)
{
    return create_window(
        window_class_name,      // class_name.
        title,                  // window_name.
        WS_OVERLAPPEDWINDOW |
        WS_CLIPCHILDREN |
        WS_CLIPSIBLINGS,        // style.
        CW_USEDEFAULT,          // x.
        CW_USEDEFAULT,          // y.
        width,                  // width.
        height,                 // height.
        HWND_DESKTOP,           // parent.
        nullptr,                // menu.
        instance,               // instance.
        param);                 // param.
}

Scoped_device_context get_device_context(_In_ HWND window)
{
    const auto device_context = GetDC(window);

    if(nullptr == device_context)
    {
        WindowsCommon::throw_hr(E_FAIL);
    }

    return make_scoped_device_context(device_context, window);
}

Scoped_gl_context create_gl_context(_In_ HDC device_context)
{
    const PIXELFORMATDESCRIPTOR descriptor =
    {
        sizeof(PIXELFORMATDESCRIPTOR),      // Size of this descriptor.
        1,                                  // Version number.
        PFD_DRAW_TO_WINDOW |                // Support window.
        PFD_SUPPORT_OPENGL |                // Support OpenGL.
        PFD_GENERIC_ACCELERATED |           // Support hardware acceleration.
        PFD_DOUBLEBUFFER,                   // Double buffered.
        PFD_TYPE_RGBA,                      // RGBA type.
        32,                                 // 32-bit color depth.
        0, 0, 0, 0, 0, 0,                   // Color bits ignored.
        0,                                  // No alpha buffer.
        0,                                  // Shift bit ignored.
        0,                                  // No accumulation buffer.
        0, 0, 0, 0,                         // Accum bits ignored.
        24,                                 // 24-bit z-buffer.
        8,                                  // 8-bit stencil buffer.
        0,                                  // No auxiliary buffer.
        PFD_MAIN_PLANE,                     // Main layer.
        0,                                  // Reserved.
        0, 0, 0                             // Layer masks ignored.
    };

    const int pixel_format = ChoosePixelFormat(device_context, &descriptor);
    if(pixel_format == 0)
    {
        throw_hr(hresult_from_last_error());
    }

    if(!SetPixelFormat(device_context, pixel_format, &descriptor))
    {
        throw_hr(hresult_from_last_error());
    }

    const auto rendering_context = wglCreateContext(device_context);
    if(nullptr == rendering_context)
    {
        throw_hr(hresult_from_last_error());
    }

    return make_scoped_gl_context(rendering_context);
}

Scoped_current_context create_current_context(_In_ HDC device_context, _In_ HGLRC gl_context)
{
    if(!wglMakeCurrent(device_context, gl_context))
    {
        throw_hr(WindowsCommon::hresult_from_last_error());
    }

    return make_scoped_current_context(gl_context);
}

}

