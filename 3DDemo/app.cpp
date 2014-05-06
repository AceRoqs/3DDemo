#include "PreCompile.h"
#include "app.h"
#include "polygon.h"
#include "coord.h"
#include "world.h"
#include "render.h"
#include "Camera.h"
#include "Action.h"
#include "DirectInputMap.h"
#include "HRException.h"
#include "InputDevice.h"
#include "ThreadAffinity.h"
#include "Tracing.h"
#include "WindowsGL.h"

static bool s_fWindowed = true;

static UINT_PTR game_message_loop(const WindowsCommon::Input_device& keyboard, const std::vector<Graphics::Polygon>& polys)
{
    // Set thread affinity to the first available processor, so that QPC
    // will always be done on the same processor.
    WindowsCommon::lock_thread_to_first_processor();

    Camera camera(0.0f, 0.0f, 1.0f, 0.0f);

    LARGE_INTEGER last_counter = {};
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    MSG message;
    for(;;)
    {
#ifdef DRAW_FRAMERATE
        DWORD dwTicks = ::GetTickCount();
#endif

        if(!WindowsCommon::dispatch_all_windows_messages(&message))
        {
            // Renderer and window handle were destroyed in WM_DESTROY.
//            assert(nullptr == m_renderer);
            break;
        }

        Camera new_camera = camera;

        const bool first_tick = (last_counter.QuadPart == 0);

        // TODO: Push QPC to input action queue.
        // TODO: Factor code into functions.
        LARGE_INTEGER current_counter;
        QueryPerformanceCounter(&current_counter);

        LARGE_INTEGER counter_diff;
        counter_diff.QuadPart = current_counter.QuadPart - last_counter.QuadPart;
        last_counter = current_counter;

        counter_diff.QuadPart *= 1000;  // Convert from seconds to milliseconds.
        const float elapsed_milliseconds = counter_diff.QuadPart / static_cast<float>(frequency.QuadPart);

        if(!first_tick)
        {
            // TODO: Think about whether to do one large update, or several fixed-size updates.
            //for(int i = 0; i < ticks; i+= 16)
            {
                WindowsCommon::dprintf("QPC: %f\r\n", elapsed_milliseconds);

                WindowsCommon::Keyboard_state keyboard_state;
                keyboard.get_input(&keyboard_state);

                std::list<Action> actions = actions_from_keyboard_state(keyboard_state);
                new_camera = apply_actions(actions, new_camera, elapsed_milliseconds);
            }
        }

        camera = new_camera;

        draw_list(polys, camera);

        const HDC device_context = wglGetCurrentDC();
        SwapBuffers(device_context);

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

        std::vector<Graphics::Polygon> polys;
        std::vector<Position_vertex> vertex_formats;
        std::vector<TexCoord> texture_coords;

        // TODO: 2014: start_load before anything else, so that async reads can happen in the background.
        // This cannot happen today, since start_load calls OpenGL to bind textures.
        start_load("polydefs.txt", &polys, &vertex_formats, &texture_coords);

        // TODO: 2014: does this need to be reinitialized if the video engine is reinitialized?
        initialize_gl_constants();
        initialize_gl_world_data(vertex_formats, texture_coords);

        ShowWindow(app.m_state.window, show_command);
        UpdateWindow(app.m_state.window);

        WindowsCommon::Input_device keyboard(instance, app.m_state.window);

        auto return_code = game_message_loop(keyboard, polys);

        // _tWinMain return code is an int type.
        assert(INT_MAX > return_code);

        // Window procedure doesn't currently have a reason to return non-zero.
        assert(0 == return_code);

        // This would be the legal part of the return code.
        //static_cast<int>(return_code);

        assert((nullptr == app.m_state.window) && "window handle was never released.");
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
        // TODO: uninitialize isn't always the correct text.
        //MessageBox(nullptr, TEXT("Unable to initialize engine."), TEXT("Exiting"), MB_OK);
        WindowsCommon::throw_hr(E_FAIL);
    }
}

