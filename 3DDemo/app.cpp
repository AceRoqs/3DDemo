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

static void append_patch_index_array(unsigned int patch_count, std::vector<uint16_t>& index_array)
{
    const auto curve_vertex_count = patch_count + 1;

    assert(index_array.size() < UINT16_MAX);
    const auto offset = static_cast<uint16_t>(index_array.size());

    // Arithmetic wrap is safe as this is just an optimization.
    index_array.reserve(offset + patch_count * patch_count * 6);     // Six push_backs per loop iteration.

    for(unsigned int vv = 0; vv < patch_count; ++vv)
    {
        for(unsigned int uu = 0; uu < patch_count; ++uu)
        {
            index_array.push_back(offset + static_cast<uint16_t>((uu + 0) + (vv + 0) * curve_vertex_count));
            index_array.push_back(offset + static_cast<uint16_t>((uu + 0) + (vv + 1) * curve_vertex_count));
            index_array.push_back(offset + static_cast<uint16_t>((uu + 1) + (vv + 0) * curve_vertex_count));
            index_array.push_back(offset + static_cast<uint16_t>((uu + 1) + (vv + 0) * curve_vertex_count));
            index_array.push_back(offset + static_cast<uint16_t>((uu + 0) + (vv + 1) * curve_vertex_count));
            index_array.push_back(offset + static_cast<uint16_t>((uu + 1) + (vv + 1) * curve_vertex_count));
        }
    }
}

static void generate_patch_index_array(unsigned int patch_count, uint16_t* begin, uint16_t* end, size_t offset)
{
    const auto curve_vertex_count = patch_count + 1;

    assert(end > begin);
    assert((end - begin) >= (patch_count * patch_count * 6));
    assert(offset + patch_count + patch_count * curve_vertex_count <= UINT16_MAX);
    (void)end;

    size_t index = 0;
    for(unsigned int vv = 0; vv < patch_count; ++vv)
    {
        for(unsigned int uu = 0; uu < patch_count; ++uu)
        {
            begin[index++] = static_cast<uint16_t>(offset + (uu + 0) + (vv + 0) * curve_vertex_count);
            begin[index++] = static_cast<uint16_t>(offset + (uu + 0) + (vv + 1) * curve_vertex_count);
            begin[index++] = static_cast<uint16_t>(offset + (uu + 1) + (vv + 0) * curve_vertex_count);
            begin[index++] = static_cast<uint16_t>(offset + (uu + 1) + (vv + 0) * curve_vertex_count);
            begin[index++] = static_cast<uint16_t>(offset + (uu + 0) + (vv + 1) * curve_vertex_count);
            begin[index++] = static_cast<uint16_t>(offset + (uu + 1) + (vv + 1) * curve_vertex_count);
        }
    }
}

static void append_patch_texture_coords_array(unsigned int patch_count, std::vector<Vector2f>& texture_coords)
{
    const float scale = 1.0f / patch_count;
    const auto curve_vertex_count = patch_count + 1;

    // Arithmetic wrap is safe as this is just an optimization.
    texture_coords.reserve(texture_coords.size() + curve_vertex_count * curve_vertex_count);

    for(unsigned int vv = 0; vv < curve_vertex_count; ++vv)
    {
        for(unsigned int uu = 0; uu < curve_vertex_count; ++uu)
        {
            texture_coords.push_back({uu * scale, vv * scale});
        }
    }
}

// TODO: 2016: This is probably better served as taking a start pointer and end pointer, and foregoing the vector/min/max.
static void generate_patch_texture_coords_array(unsigned int patch_count, std::vector<Vector2f>& texture_coords, uint16_t min, uint16_t max)
{
    const auto curve_vertex_count = patch_count + 1;
    assert(texture_coords.size() <= UINT16_MAX);
    assert((max - min + 1u) >= (curve_vertex_count * curve_vertex_count));
    assert(max < texture_coords.size());
    (void)max;

    const float scale = 1.0f / patch_count;

    size_t offset = min;
    for(unsigned int vv = 0; vv < curve_vertex_count; ++vv)
    {
        for(unsigned int uu = 0; uu < curve_vertex_count; ++uu)
        {
            texture_coords[vv * curve_vertex_count + uu + offset++] = {uu * scale, vv * scale};
        }
    }
}

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

        // TODO: 2016: index_array and texture_coords (and vertices) can be cached, as long as patch_count doesn't change between frames.
        // TODO: 2016: When reusing arrays, can precalculate the reserve to be the largest expected size, so reallocation never happens.
        Patch patch1;
        append_quadratic_bezier_vertex_patch(patches[0], patch_count, patch1.vertices);
        append_patch_index_array(patch_count, patch1.index_array);
        append_patch_texture_coords_array(patch_count, patch1.texture_coords);
        patch1.patch_count = patch_count;
        patch1.texture_id = map.patch_texture_id;
        patch1.index_array_offset = 0;

        Patch patch2;
        append_quadratic_bezier_vertex_patch(patches[1], patch_count, patch2.vertices);
        append_patch_index_array(patch_count, patch2.index_array);
        append_patch_texture_coords_array(patch_count, patch2.texture_coords);
        patch2.patch_count = patch_count;
        patch2.texture_id = map.patch_texture_id;
        patch2.index_array_offset = 0;

        emitter.update(elapsed_milliseconds);

        draw_map(map, camera, patch1, patch2, emitter);

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

