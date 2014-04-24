#include "PreCompile.h"
#include "input.h"          // Pick up forward declarations to ensure correctness.
#include "HRException.h"
#include "Camera.h"
#include "Tracing.h"
#include "world.h"      // Temp: input.cpp should not do action mapping.
#include "Action.h"

#define KEYBOARD_BUFFER_SIZE 256

//---------------------------------------------------------------------------
Input_device::Input_device(
    HINSTANCE hInstance,
    HWND hwnd)
{
    // Create DirectInput keyboard device.
    using namespace WindowsCommon;

    ATL::CComPtr<IDirectInput8> direct_input;
    throw_hr(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<PVOID*>(&direct_input), nullptr));
    throw_hr(direct_input->CreateDevice(GUID_SysKeyboard, &m_device, nullptr));

    assert(c_dfDIKeyboard.dwDataSize == KEYBOARD_BUFFER_SIZE);
    throw_hr(m_device->SetDataFormat(&c_dfDIKeyboard));
    throw_hr(m_device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

    // Acquisition is done before input is read the first time.

    direct_input.Release();
}

//---------------------------------------------------------------------------
Input_device::~Input_device()
{
    m_device->Unacquire();
}

// TODO: 2014: Come up with a new name since "action map" is also a DirectInput concept.
const static struct Action_map
{
    int input;
    Action action;
} action_map[] =
{
    DIK_NUMPAD2, Move_backward,
    DIK_DOWN,    Move_backward,
    DIK_NUMPAD8, Move_forward,
    DIK_UP,      Move_forward,
    DIK_D,       Strafe_right,
    DIK_A,       Strafe_left,
    DIK_NUMPAD4, Turn_left,
    DIK_LEFT,    Turn_left,
    DIK_NUMPAD6, Turn_right,
    DIK_RIGHT,   Turn_right,
};

static Camera apply_actions(const std::list<Action>& actions, const Camera& camera, DWORD ticks)
{
    Camera new_camera = camera;
    float new_x = new_camera.m_x;
    float new_z = new_camera.m_z;

    // These two values can be configurable for gameplay.
    const auto walk_distance_per_tick = 0.045f;
    const auto keyboard_rotational_speed_per_tick = 0.5f;

    const auto walk_distance = walk_distance_per_tick * ticks;
    const auto radians_per_degree = static_cast<float>(M_PI * 2.0f / 360.0f);
    const auto sine = sinf(new_camera.m_degrees * radians_per_degree) * walk_distance;
    const auto cosine = cosf(new_camera.m_degrees * radians_per_degree) * walk_distance;
    const auto rotation_degrees = keyboard_rotational_speed_per_tick * ticks;

    // Accumulate all movement inputs before application.  This will prevent
    // two contractory button presses (e.g. strafe and forward) from bouncing
    // between each other in, say, a corner.
    // TODO: There still exists an issue, where pushing into a corner, and
    // then moving forward and strafing at the same time still produces movement.
    // Ideally, moving forward would just follow the edge until there is no possible
    // forward motion possible, even while moving laterally.
    for(auto action = actions.cbegin(); action != actions.cend(); ++action)
    {
        switch(*action)
        {
            case Move_forward:
            {
                new_x -= sine;
                new_z += cosine;
                break;
            }

            case Move_backward:
            {
                new_x += sine;
                new_z -= cosine;
                break;
            }

            case Strafe_right:
            {
                new_x -= cosine;
                new_z -= sine;
                break;
            }

            case Strafe_left:
            {
                new_x += cosine;
                new_z += sine;
                break;
            }

            case Turn_right:
            {
                new_camera.m_degrees = fmod(new_camera.m_degrees + rotation_degrees, 360.f);
                break;
            }

            case Turn_left:
            {
                new_camera.m_degrees = fmod(new_camera.m_degrees - rotation_degrees + 360.f, 360.f);
                break;
            }
        }
    }

    // Testing each coordinate on its own only is correct because each boundary is axis aligned.
    float updated_x = new_camera.m_x;
    if(is_point_in_world(new_x, new_camera.m_y, new_camera.m_z))
    {
        updated_x = new_x;
    }
    if(is_point_in_world(new_camera.m_x, new_camera.m_y, new_z))
    {
        new_camera.m_z = new_z;
    }
    new_camera.m_x = updated_x;

    return new_camera;
}

typedef std::array<uint8_t, KEYBOARD_BUFFER_SIZE> Keyboard_state;
static std::list<Action> actions_from_keyboard_state(const Keyboard_state& keyboard_state)
{
    // TODO: 2014: should this be an input map of all inputs (mouse, keyboard, network, time)
    // rather than an action map?
    std::list<Action> actions;
    for(int ix = 0; ix < ARRAYSIZE(action_map); ++ix)
    {
        if(keyboard_state[action_map[ix].input])
        {
            actions.push_back(action_map[ix].action);
        }
    }

    actions.sort();
    actions.unique();

    return actions;
}

Camera Input_device::get_input(const Camera& camera)
{
    static long msec = 0;

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

            std::list<Action> actions;
            if(SUCCEEDED(m_device->Acquire()))
            {
                Keyboard_state keyboard_state;
                WindowsCommon::throw_hr(m_device->GetDeviceState(keyboard_state.size(), &keyboard_state));

                actions = actions_from_keyboard_state(keyboard_state);
            }

            new_camera = apply_actions(actions, new_camera, ticks);
        }
    }

    return new_camera;
}

