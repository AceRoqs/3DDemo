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

// TODO: 2016: map was const until emitters became mutable.  See if all mutable state can be pushed to a
// secondary struct, so map can become const again.
static UINT_PTR game_message_loop(Map& map, WindowsCommon::Clock& clock, const WindowsCommon::Input_device& keyboard)
{
    Camera camera(make_vector(0.0f, 0.0f, 1.0f), 0.0f);

    const unsigned int MAX_PATCH_COUNT_PER_DIMENSION = 9;
    const unsigned int MAX_GENERATED_VERTICES_PER_DIMENSION = MAX_PATCH_COUNT_PER_DIMENSION + 1;
    const unsigned int MAX_GENERATED_VERTICES = MAX_GENERATED_VERTICES_PER_DIMENSION * MAX_GENERATED_VERTICES_PER_DIMENSION;
    const unsigned int MAX_GENERATED_INDICES_PER_DIMENSION = MAX_PATCH_COUNT_PER_DIMENSION;
    const unsigned int MAX_GENERATED_INDICES = MAX_GENERATED_INDICES_PER_DIMENSION * MAX_GENERATED_INDICES_PER_DIMENSION * 6;

    // Allocate the maximum size so reallocation never happens.
    const auto dynamic_mesh_count = map.implicit_surfaces.size();
    Dynamic_meshes dynamic_meshes;
    dynamic_meshes.vertex_array.resize(MAX_GENERATED_VERTICES * dynamic_mesh_count);
    dynamic_meshes.texture_coords_array.resize(MAX_GENERATED_VERTICES * dynamic_mesh_count);
    dynamic_meshes.index_array.resize(MAX_GENERATED_INDICES * dynamic_mesh_count);
    dynamic_meshes.implicit_surfaces.resize(dynamic_mesh_count);

    // Initialize implicit surface data.
    for(auto ii = 0u; ii < dynamic_mesh_count; ++ii)
    {
        dynamic_meshes.implicit_surfaces[ii].patch_count = MAX_PATCH_COUNT_PER_DIMENSION + 1;
        dynamic_meshes.implicit_surfaces[ii].index_array_offset = ii * MAX_GENERATED_INDICES;
    }

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

        for(auto ii = 0u; ii < dynamic_mesh_count; ++ii)
        {
            // Set level-of-detail.
            unsigned int patch_count = (unsigned int)(MAX_GENERATED_VERTICES_PER_DIMENSION * 4 / (point_distance(camera.m_position, map.implicit_surfaces[ii].origin))) - 1;
            patch_count = std::min(std::max(2u, patch_count), MAX_PATCH_COUNT_PER_DIMENSION);

            if(dynamic_meshes.implicit_surfaces[ii].patch_count != patch_count)
            {
                dynamic_meshes.implicit_surfaces[ii].patch_count = patch_count;

                const auto vertex_array = generate_quadratic_bezier_vertex_array(map.implicit_surfaces[ii].control_points, patch_count);
                std::copy(std::cbegin(vertex_array), std::cend(vertex_array), std::begin(dynamic_meshes.vertex_array) + MAX_GENERATED_VERTICES * ii);

                const auto texture_coords_array = generate_implicit_surface_texture_coords_array(patch_count);
                std::copy(std::cbegin(texture_coords_array), std::cend(texture_coords_array), std::begin(dynamic_meshes.texture_coords_array) + MAX_GENERATED_VERTICES * ii);

                const auto index_array = generate_implicit_surface_index_array(patch_count, MAX_GENERATED_VERTICES * ii);
                std::copy(std::cbegin(index_array), std::cend(index_array), std::begin(dynamic_meshes.index_array) + MAX_GENERATED_INDICES * ii);
            }
        }

        // TODO: 2016: Update all emitters, and don't pass emitters explicitly to draw_map.
        map.emitters[0].update(elapsed_milliseconds);

        draw_map(map, dynamic_meshes, camera, map.emitters[0]);

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
#ifndef NDEBUG
    WindowsCommon::Scoped_FPU_exception_control fpu(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID);
    fpu.enable(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID);
    auto current_control = fpu.current_control();
#endif

    // Start load first, to kick off async reads.
    std::vector<ImageProcessing::Bitmap> texture_list;
    Map map = start_load("polydefs.txt", &texture_list);

    App_window app(instance, true);

    // TODO: 2014: does this need to be reinitialized if the video engine is reinitialized?
    initialize_gl_constants();
    initialize_gl_world_data(texture_list);

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

}

