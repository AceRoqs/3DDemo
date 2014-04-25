#include "PreCompile.h"
#include "input.h"          // Pick up forward declarations to ensure correctness.
#include "HRException.h"
#include "Camera.h"
#include "Tracing.h"
#include "Action.h"

Input_device::Input_device(_In_ HINSTANCE hInstance, _In_ HWND hwnd)
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

Input_device::~Input_device()
{
    m_device->Unacquire();
}

void Input_device::get_input(_In_ Keyboard_state* keyboard_state)
{
    if(SUCCEEDED(m_device->Acquire()))
    {
        WindowsCommon::throw_hr(m_device->GetDeviceState(keyboard_state->size(), keyboard_state));
    }
}

// TODO: 2014: Come up with a new name since "action map" is also a DirectInput concept.
// TODO: 2014: Can't move this to Action.cpp yet, since Action.cpp should eventually be platform
// agnostic, and not tied to the DirectInput enums below.
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

std::list<Action> actions_from_keyboard_state(const Keyboard_state& keyboard_state)
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

