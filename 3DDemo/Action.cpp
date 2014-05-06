#include "PreCompile.h"
#include "Action.h"
#include "Camera.h"
#include "world.h"

Camera apply_actions(const std::list<Action>& actions, const Camera& camera, float milliseconds)
{
    Camera new_camera = camera;
    float new_x = new_camera.m_x;
    float new_z = new_camera.m_z;

    // These two values can be configurable for gameplay.
    const auto walk_distance_per_tick = 0.045f;
    const auto keyboard_rotational_speed_per_tick = 0.5f;

    const auto walk_distance = walk_distance_per_tick * milliseconds;
    const auto radians_per_degree = static_cast<float>(M_PI * 2.0f / 360.0f);
    const auto sine = sinf(new_camera.m_degrees * radians_per_degree) * walk_distance;
    const auto cosine = cosf(new_camera.m_degrees * radians_per_degree) * walk_distance;
    const auto rotation_degrees = keyboard_rotational_speed_per_tick * milliseconds;

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

