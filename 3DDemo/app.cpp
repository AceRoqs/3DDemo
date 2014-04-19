#include "PreCompile.h"
#include "app.h"
#include "polygon.h"
#include "input.h"
#include "coord.h"
#include "WindowsGL.h"
#include "world.h"
#include "render.h"
#include "HRException.h"

static bool s_fWindowed = true;

static int game_message_loop(std::function<void(void)> execute_frame)
{
    MSG message;
    for(;;)
    {
#ifdef DRAW_FRAMERATE
        DWORD dwTicks = ::GetTickCount();
#endif

        // Clear out all the messages before drawing a new frame.
        BOOL message_exists = PeekMessage(&message, nullptr, 0, 0, PM_REMOVE);
        while(message_exists)
        {
            if(WM_QUIT == message.message)
            {
                break;
            }

            ::TranslateMessage(&message);
            DispatchMessage(&message);

            message_exists = PeekMessage(&message, nullptr, 0, 0, PM_REMOVE);
        }

        if(message_exists && (WM_QUIT == message.message))
        {
            // Renderer and window handle were destroyed in WM_DESTROY.
//            assert(nullptr == m_renderer);
            break;
        }

        try
        {
            execute_frame();

#ifdef DRAW_FRAMERATE
            dwTicks = ::GetTickCount() - dwTicks;

            TCHAR a[] = TEXT("frames/sec:     ");
            HDC hdc = GetDC(window);
            ::TextOut(hdc, 0, 0, a, lstrlen(a));
            _itot_s(DWORD(1.0f / (float(dwTicks) / 1000.0f)), a+12, 5, 10);
            ::SetBkColor(hdc, 0);
            ::SetTextColor(hdc, 0xffffff);
            ::TextOut(hdc, 0, 0, a, lstrlen(a));
            ReleaseDC(window, hdc);
#endif
        }
        // Convert exceptions to a OS native error type for later display.
        catch(const WindowsCommon::HRESULT_exception& ex)
        {
            UNREFERENCED_PARAMETER(ex);

            throw;
        }
        catch(const std::bad_alloc& ex)
        {
            UNREFERENCED_PARAMETER(ex);

            WindowsCommon::throw_hr(E_OUTOFMEMORY);
        }
        catch(...)
        {
            WindowsCommon::throw_hr(E_FAIL);
        }
    }

    return message.wParam;
}

namespace WindowsCommon
{

class App_window_procedure : public WindowsCommon::WindowGL_window_procedure
{
public:
    App_window_procedure(_In_ HINSTANCE instance, bool windowed) : WindowGL_window_procedure(instance, windowed)
    {
    }

protected:
    LRESULT window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param) NOEXCEPT
    {
        LRESULT return_value = WindowGL_window_procedure::window_proc(window, message, w_param, l_param);

        if(message == WM_DESTROY)
        {
            PostQuitMessage(0);
        }

        return return_value;
    }
};

void app_run(HINSTANCE instance, int show_command)
{
    try
    {
        App_window_procedure app(instance, true);
        Input_device keyboard(instance, app.m_state.window);

        // TODO: 2014: does this need to be reinitialized if the video engine is reinitialized?
        initialize_gl_constants();

        std::vector<CPolygon> polys;
        std::vector<Position_vertex> vertex_formats;
        std::vector<TexCoord> texture_coords;

        start_load("polydefs.txt", &polys, &vertex_formats, &texture_coords);

        initialize_gl_world_data(vertex_formats, texture_coords);

        float camera_x = 0.0f;
        float camera_y = 0.0f;
        float camera_z = 1.0f;
        float camera_degrees = 0.0f;

        ShowWindow(app.m_state.window, show_command);
        UpdateWindow(app.m_state.window);

        // Lambda requires copy constructor, which Scoped_device_context does not provide.
        const HDC device_context = app.m_state.device_context;
        auto execute_frame = [&, device_context]()
        {
            keyboard.get_input(&camera_x, &camera_y, &camera_z, &camera_degrees);
            draw_list([=](){ SwapBuffers(device_context); }, polys, camera_x, camera_y, camera_z, camera_degrees);
        };

        int return_code = game_message_loop(execute_frame);
        (return_code);

        assert(!IsWindow(app.m_state.window));
    }
    catch(...)
    {
        // TODO: uninitialize isn't always the correct text.
        MessageBox(nullptr, TEXT("Unable to initialize engine."), TEXT("Exiting"), MB_OK);
    }
}

}

