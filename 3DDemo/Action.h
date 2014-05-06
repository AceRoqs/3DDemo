#ifndef ACTION_H
#define ACTION_H

// TODO: namespace

enum Action
{
    Move_forward,
    Move_backward,
    Strafe_right,
    Strafe_left,
    Turn_right,
    Turn_left
};

struct Camera apply_actions(const std::list<Action>& actions, const struct Camera& camera, float milliseconds);

#endif

