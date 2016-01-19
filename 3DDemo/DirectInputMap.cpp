#include "PreCompile.h"
#include "DirectInputMap.h"
#include "Action.h"

namespace Demo
{

// TODO: 2014: Come up with a new name since "action map" is also a DirectInput concept.
// TODO: 2014: Can't move this to Action.cpp yet, since Action.cpp should eventually be platform
// agnostic, and not tied to the DirectInput enums below.
const static struct Action_map
{
    int input;
    Action action;
} action_map[] =
{
    DIK_NUMPAD2, Action::Move_backward,
    DIK_DOWN,    Action::Move_backward,
    DIK_NUMPAD8, Action::Move_forward,
    DIK_UP,      Action::Move_forward,
    DIK_D,       Action::Strafe_right,
    DIK_A,       Action::Strafe_left,
    DIK_NUMPAD4, Action::Turn_left,
    DIK_LEFT,    Action::Turn_left,
    DIK_NUMPAD6, Action::Turn_right,
    DIK_RIGHT,   Action::Turn_right,
};

std::vector<std::pair<float, Action>> actions_from_keyboard_state(float ellapsed_milliseconds, const WindowsCommon::Keyboard_state& keyboard_state)
{
    // TODO: 2014: should this be an input map of all inputs (mouse, keyboard, network, time)
    // rather than an action map?
    std::vector<std::pair<float, Action>> actions;
    std::for_each(&action_map[0], &action_map[0] + ARRAYSIZE(action_map), [&](const Action_map& map)
    {
        if(keyboard_state[map.input])
        {
            actions.push_back(std::make_pair(ellapsed_milliseconds, map.action));
        }
    });

    std::sort(std::begin(actions), std::end(actions));
    std::unique(std::begin(actions), std::end(actions));

    return actions;
}

// Experimental Xbox 360 controller code.
#if 0
#include <XInput.h>

std::vector<std::pair<float, Action>> actions_from_joystick_state(float ellapsed_milliseconds)
{
    std::vector<std::pair<float, Action>> actions;

    XINPUT_STATE state = {};
    DWORD dwResult = XInputGetState(0, &state);
    if(dwResult != ERROR_DEVICE_NOT_CONNECTED)
    {
        if((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0)
        {
            actions.push_back(std::make_pair(ellapsed_milliseconds, Move_forward));
        }
        if((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0)
        {
            actions.push_back(std::make_pair(ellapsed_milliseconds, Move_backward));
        }
        if((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0)
        {
            actions.push_back(std::make_pair(ellapsed_milliseconds, Strafe_right));
        }
        if((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0)
        {
            actions.push_back(std::make_pair(ellapsed_milliseconds, Strafe_left));
        }
        if(state.Gamepad.bLeftTrigger > 50)
        {
            actions.push_back(std::make_pair(ellapsed_milliseconds, Turn_left));
        }
        if(state.Gamepad.bRightTrigger > 50)
        {
            actions.push_back(std::make_pair(ellapsed_milliseconds, Turn_right));
        }
    }

    return actions;
}
#endif

}

