#pragma once

// TODO: can this header be removed somehow?
#include "InputDevice.h"

namespace Demo
{

std::vector<std::pair<float, enum class Action>> actions_from_keyboard_state(float ellapsed_milliseconds, const WindowsCommon::Keyboard_state& keyboard_state);

}

