#include "PreCompile.h"
#include "app.h"
#include "world.h"
#include "render.h"
#include "particle.h"
#include "Camera.h"
#include "Action.h"
#include "GenerateMesh.h"
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
static UINT_PTR game_message_loop(Map& map, const std::vector<unsigned int>& texture_ids, WindowsCommon::Clock& clock, const WindowsCommon::Input_device& keyboard)
{
    Camera camera {make_vector(0.0f, 0.0f, 1.0f), 0.0f};

    constexpr unsigned int MAX_PATCH_COUNT_PER_DIMENSION = 9;
    constexpr unsigned int MAX_GENERATED_VERTICES_PER_DIMENSION = MAX_PATCH_COUNT_PER_DIMENSION + 1;
    constexpr unsigned int MAX_GENERATED_VERTICES = MAX_GENERATED_VERTICES_PER_DIMENSION * MAX_GENERATED_VERTICES_PER_DIMENSION;
    constexpr unsigned int MAX_GENERATED_INDICES_PER_DIMENSION = MAX_PATCH_COUNT_PER_DIMENSION;
    constexpr unsigned int MAX_GENERATED_INDICES = MAX_GENERATED_INDICES_PER_DIMENSION * MAX_GENERATED_INDICES_PER_DIMENSION * 6;

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
        PortableRuntime::dprintf("QPC: %f ms\n", elapsed_milliseconds);

        WindowsCommon::Keyboard_state keyboard_state;
        keyboard.get_input(&keyboard_state);

        const std::vector<std::pair<float, Action>> actions = actions_from_keyboard_state(elapsed_milliseconds, keyboard_state);
        camera = apply_actions(actions, camera);

        for(auto ii = 0u; ii < dynamic_mesh_count; ++ii)
        {
            // Set level-of-detail.
            // TODO: 2016: This is a single LOD for the entire implicit surface, even if the surface could span LOD levels.
            unsigned int patch_count = (unsigned int)(MAX_GENERATED_VERTICES_PER_DIMENSION * 4 / (point_distance(camera.m_position, map.implicit_surfaces[ii].origin))) - 1;
            patch_count = std::min(std::max(2u, patch_count), MAX_PATCH_COUNT_PER_DIMENSION);

            if(dynamic_meshes.implicit_surfaces[ii].patch_count != patch_count)
            {
                dynamic_meshes.implicit_surfaces[ii].patch_count = patch_count;

                // TODO: 2016: One option is to have one global texture coords array for each LOD level,
                // and generate that once.  That might be a size/speed tradeoff, and might not even be a
                // size tradeoff... However, this only works when using a single LOD for the whole surface.
                generate_quadratic_bezier_vertex_array(map.implicit_surfaces[ii].control_points, patch_count, std::begin(dynamic_meshes.vertex_array) + MAX_GENERATED_VERTICES * ii);
                generate_implicit_surface_texture_coords_array(patch_count, std::begin(dynamic_meshes.texture_coords_array) + MAX_GENERATED_VERTICES * ii);
                generate_implicit_surface_index_array(patch_count, MAX_GENERATED_VERTICES * ii, std::begin(dynamic_meshes.index_array) + MAX_GENERATED_INDICES * ii);
            }
        }

        std::for_each(std::begin(map.emitters), std::end(map.emitters), [elapsed_milliseconds](Emitter& emitter)
        {
            emitter.update(elapsed_milliseconds);
        });

        Renderer::draw_map(map, texture_ids, dynamic_meshes, camera);

        const HDC device_context = wglGetCurrentDC();
        SwapBuffers(device_context);
    }

    return message.wParam;
}

class App_window : public WindowsCommon::OpenGL_window
{
protected:
    int m_reference_x{};
    int m_accumulated_x{};

    virtual LRESULT window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param) noexcept override
    {
        LRESULT return_value = OpenGL_window::window_proc(window, message, w_param, l_param);

        if(message == WM_MOUSEMOVE)
        {
            auto x_pos = GET_X_LPARAM(l_param);

            // Assume mouse look requires a button for now.
            // TODO: 2016: Think about how this behaves if the mouse button is down while the cursor is moved into the window.
            if((w_param & MK_LBUTTON) != 0)
            {
                auto diff = x_pos - m_reference_x;
                m_accumulated_x += diff;

                PortableRuntime::dprintf("Accumulated X: %d\n", m_accumulated_x);
            }

            m_reference_x = x_pos;
        }
        else if(message == WM_DESTROY)
        {
            PostQuitMessage(0);
        }

        return return_value;
    }

public:
    App_window(_In_ HINSTANCE instance, bool windowed) :
        OpenGL_window(u8"3D Demo 1999 (Updated for C++11)", instance, windowed)
    {
    }
};

void app_run(_In_ HINSTANCE instance, int show_command)
{
#ifndef NDEBUG
    WindowsCommon::Scoped_FPU_exception_control fpu(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID);
    fpu.enable(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID);
    const auto current_control = fpu.current_control();
#endif

    WindowsCommon::dprintf_system_cpu_set_information();

    // Start load first, to kick off async reads.
    Map map = start_load("polydefs.txt");

    App_window app(instance, true);
    Renderer renderer;

    const auto texture_ids = Renderer::initialize_gl_world_data(map.texture_list);

    // Set thread affinity to the first available processor, so that QPC
    // will always be done on the same processor.
    WindowsCommon::lock_thread_to_first_processor();
    WindowsCommon::Clock clock;

    WindowsCommon::Input_device keyboard(instance, app.window());

    ShowWindow(app.window(), show_command);
    UpdateWindow(app.window());

    WindowsCommon::debug_validate_message_map();
    const auto return_code = game_message_loop(map, texture_ids, clock, keyboard);

    // TODO: 2016: This is not called if an exception happens.
    Renderer::deinitialize_gl_world_data(texture_ids);

#ifndef NDEBUG
    assert(fpu.current_control() == current_control);
#endif

    // _tWinMain return code is an int type.
    assert(INT_MAX > return_code);

    // Window procedure doesn't currently have a reason to return non-zero.
    assert(0 == return_code);

    // This would be the legal part of the return code.
    //static_cast<int>(return_code);
    (void)return_code;  // Unreferenced variable.

    assert((nullptr == app.window()) && "window handle was never released.");
}

}

