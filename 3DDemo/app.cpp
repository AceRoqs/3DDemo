#include "PreCompile.h"
#include "app.h"
#include "polygon.h"
#include "input.h"
#include "coord.h"
#include "video.h"
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
//            assert(!IsWindow(window.get()));
//            window.release();
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

void Frame_app::app_run(HINSTANCE instance, int show_command)
{
    auto state = Startup_Video(instance, true);
    // TODO: 2014: exception, not null atom, should be thrown.
    if(!state.atom)
    {
        MessageBox(nullptr, TEXT("Unable to initialize engine."), TEXT("Exiting"), MB_OK);
        return;
    }

    Input_device keyboard(instance, state.window);

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

    ShowWindow(state.window, show_command);
    UpdateWindow(state.window);

    // Lambda requires copy constructor, which Scoped_device_context does not provide.
    const HDC device_context = state.device_context;
    auto execute_frame = [&, device_context]()
    {
        keyboard.get_input(&camera_x, &camera_y, &camera_z, &camera_degrees);
        draw_list([=](){ SwapBuffers(device_context); }, polys, camera_x, camera_y, camera_z, camera_degrees);
    };

    int return_code = game_message_loop(execute_frame);
    (return_code);

    // EndEngine(window, device_context);
    Shutdown_Video(s_fWindowed, state);

    // TODO: 2014: release contents, since they are invalid.  These should just be handled by destructor..
    state.device_context.release();
    state.window.release();
}

}

