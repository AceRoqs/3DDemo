#include "PreCompile.h"
#include "app.h"
#include "polygon.h"
#include "input.h"
#include "coord.h"
#include "WindowsGL.h"
#include "world.h"
#include "render.h"
#include "HRException.h"
#include "Camera.h"
#include "Tracing.h"
#include "Action.h"
#include "DirectInputMap.h"

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

class App_window_procedure : public WindowsCommon::WindowGL_window_procedure
{
public:
    App_window_procedure(_In_ HINSTANCE instance, bool windowed) :
        WindowGL_window_procedure(TEXT("3D Demo 1999 (Updated for C++11)"), instance, windowed)
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

        Camera camera(0.0f, 0.0f, 1.0f, 0.0f);

        ShowWindow(app.m_state.window, show_command);
        UpdateWindow(app.m_state.window);

        long msec = 0;

        // Lambda requires copy constructor, which Scoped_device_context does not provide.
        const HDC device_context = app.m_state.device_context;
        auto execute_frame = [&, device_context]()
        {
            Camera new_camera = camera;

            // TODO: get number of times from system queue
            // TODO: this is a bunch of crap
            const auto tick_count = GetTickCount();
            const bool first_tick = (msec == 0);
            const auto ticks = tick_count - msec;
            // TODO: send data to system queue instead of moving camera
            msec = tick_count;

            if(!first_tick)
            {
                //for(int i = 0; i < ticks; i+= 16) 
                {
                    WindowsCommon::dprintf("tick_count - msec: %d\r\n", ticks);

                    Keyboard_state keyboard_state;
                    keyboard.get_input(&keyboard_state);

                    std::list<Action> actions = actions_from_keyboard_state(keyboard_state);
                    new_camera = apply_actions(actions, new_camera, ticks);
                }
            }

            camera = new_camera;

            draw_list([=](){ SwapBuffers(device_context); }, polys, camera);
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

