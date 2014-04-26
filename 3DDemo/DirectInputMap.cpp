#include "PreCompile.h"
#include "DirectInputMap.h"
#include "Action.h"

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

