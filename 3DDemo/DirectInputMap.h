#ifndef DIRECTINPUTMAP_H
#define DIRECTINPUTMAP_H

// TODO: can this header be removed somehow?
#include "input.h"

std::list<enum Action> actions_from_keyboard_state(const Keyboard_state& keyboard_state);

#endif

