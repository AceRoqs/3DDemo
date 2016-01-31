#include "PreCompile.h"
#include "app.h"
#include "world.h"
#include "render.h"
#include "particle.h"
#include "Camera.h"
#include "Action.h"
#include "Bezier.h"
#include "DirectInputMap.h"
#include "LinearAlgebra.h"
#include <ImageProcessing/Bitmap.h>
#include <WindowsCommon/FPU.h>
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/Clock.h>
#include <WindowsCommon/InputDevice.h>
#include <WindowsCommon/ThreadAffinity.h>
#include <WindowsCommon/WindowMessages.h>
#include <WindowsCommon/WindowsGL.h>
#include <PortableRuntime/Tracing.h>

namespace Demo
{

static bool s_fWindowed = true;

static const Particle_descriptor particle_descriptor =
{
    // Position.
    0.5f,       // x scale.
    -0.25f,     // x bias.
    0.0f,       // y scale.
    0.0f,       // y bias.
    0.0f,       // z scale.
    0.0f,       // z bias.

    // Velocity.
    -0.004f,    // x scale.
    0.0016f,    // x bias.
    0.008f,     // y scale.
    0.0f,       // y bias.
    -0.004f,    // z scale.
    0.0016f,    // z bias.

    150.0f,     // life.
};

static const Vector3f bezier_control_points[] =
{
    { -2.0f, 0.0f, -10.0f },    // 0
    { -2.0f, 0.0f, -11.0f },    // 1
    { -3.0f, 0.0f, -11.0f },    // 2
    { -2.0f,-1.0f, -10.0f },    // 3
    { -2.0f,-1.0f, -11.0f },    // 4
    { -3.0f,-1.0f, -11.0f },    // 5
    { -2.0f,-2.0f, -10.0f },    // 6
    { -2.0f,-2.0f, -11.0f },    // 7
    { -3.0f,-2.0f, -11.0f },    // 8
    { -4.0f, 0.0f, -11.0f },    // 9
    { -4.0f, 0.0f, -10.0f },    // 10
    { -4.0f,-1.0f, -11.0f },    // 11
    { -4.0f,-1.0f, -10.0f },    // 12
    { -4.0f,-2.0f, -11.0f },    // 13
    { -4.0f,-2.0f, -10.0f },    // 14
};

static const Bezier_patch patches[] =
{
    { { bezier_control_points }, 0,  1,  2,  3,  4,  5,  6,  7,  8 },
    { { bezier_control_points }, 2,  9, 10,  5, 11, 12,  8, 13, 14 },
};

static UINT_PTR game_message_loop(const Map& map, WindowsCommon::Clock& clock, const WindowsCommon::Input_device& keyboard)
{
    Camera camera(make_vector(0.0f, 0.0f, 1.0f), 0.0f);
    Emitter emitter(make_vector(-3.0f, 0.0f, -10.5f), 50, particle_descriptor);

    MSG message;
    for(;;)
    {
        if(!WindowsCommon::dispatch_all_windows_messages(&message))
        {
            // Renderer and window handle were destroyed in WM_DESTROY.
//            assert(nullptr == m_renderer);
            break;
        }

        const float elapsed_milliseconds = clock.ellapsed_milliseconds();
        PortableRuntime::dprintf("QPC: %f\n", elapsed_milliseconds);

        WindowsCommon::Keyboard_state keyboard_state;
        keyboard.get_input(&keyboard_state);

        const std::vector<std::pair<float, Action>> actions = actions_from_keyboard_state(elapsed_milliseconds, keyboard_state);
        camera = apply_actions(actions, camera);

        // Set level-of-detail.
        const unsigned int MAX_GENERATED_POINTS = 10;
        unsigned int patch_count = (unsigned int)(MAX_GENERATED_POINTS * 4 / (point_distance(camera.m_position, make_vector(2.0f, 0.0f, 10.0f)))) - 1;
        patch_count = std::min(std::max(2u, patch_count), MAX_GENERATED_POINTS - 1);
        std::vector<Vector3f> vertices = generate_quadratic_bezier_quads(patches[0], patch_count);
        std::vector<Vector3f> vertices2 = generate_quadratic_bezier_quads(patches[1], patch_count);

        emitter.update(elapsed_milliseconds);

        draw_list(map, vertices, vertices2, patch_count, 2, emitter, camera);

        const HDC device_context = wglGetCurrentDC();
        SwapBuffers(device_context);
    }

    return message.wParam;
}

class App_window : public WindowsCommon::OpenGL_window
{
public:
    App_window(_In_ HINSTANCE instance, bool windowed) :
        OpenGL_window(u8"3D Demo 1999 (Updated for C++11)", instance, windowed)
    {
    }

protected:
    LRESULT window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param) noexcept override
    {
        LRESULT return_value = OpenGL_window::window_proc(window, message, w_param, l_param);

        if(message == WM_DESTROY)
        {
            PostQuitMessage(0);
        }

        return return_value;
    }
};

void app_run(_In_ HINSTANCE instance, int show_command)
{
    try
    {
#ifndef NDEBUG
        WindowsCommon::Scoped_FPU_exception_control fpu(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID);
        fpu.enable(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID);
        auto current_control = fpu.current_control();
#endif

        // Start load first, to kick off async reads.
        std::vector<ImageProcessing::Bitmap> texture_list;
        std::vector<Vector3f> vertices;
        std::vector<Vector2f> texture_coords;
        Map map = start_load("polydefs.txt", &texture_list, &vertices, &texture_coords);

        App_window app(instance, true);

        // TODO: 2014: does this need to be reinitialized if the video engine is reinitialized?
        initialize_gl_constants();
        initialize_gl_world_data(texture_list, vertices, texture_coords);

        // Set thread affinity to the first available processor, so that QPC
        // will always be done on the same processor.
        WindowsCommon::lock_thread_to_first_processor();
        WindowsCommon::Clock clock;

        WindowsCommon::Input_device keyboard(instance, app.m_state.window);

        ShowWindow(app.m_state.window, show_command);
        UpdateWindow(app.m_state.window);

        WindowsCommon::debug_validate_message_map();
        auto return_code = game_message_loop(map, clock, keyboard);

#ifndef NDEBUG
        assert(fpu.current_control() == current_control);
#endif

        // _tWinMain return code is an int type.
        assert(INT_MAX > return_code);

        // Window procedure doesn't currently have a reason to return non-zero.
        assert(0 == return_code);

        // This would be the legal part of the return code.
        //static_cast<int>(return_code);
        UNREFERENCED_PARAMETER(return_code);

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

        throw WindowsCommon::HRESULT_exception(E_OUTOFMEMORY, __FILE__, __LINE__);
    }
    catch(...)
    {
        // TODO: uninitialize isn't always the correct text.
        //MessageBox(nullptr, TEXT("Unable to initialize engine."), TEXT("Exiting"), MB_OK);
        throw WindowsCommon::HRESULT_exception(E_FAIL, __FILE__, __LINE__);
    }
}

}

