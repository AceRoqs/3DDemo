#include "PreCompile.h"
#include "input.h"          // Pick up forward declarations to ensure correctness.
#include "HRException.h"
#include "Camera.h"
#include "Tracing.h"

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

//---------------------------------------------------------------------------
// returns false if the line between the new points and
// the current camera view points crosses a solid polygon.
static bool TestPolys(
    float x,
    float y,
    float z)
{
    (y);    // unreferenced parameter

//    return true;
    // TODO: finishme
    if(x < -9.0 || x > 9.0)
    {
        return false;
    }
    if(z < 1.0 || z > 19.0)
    {
        return false;
    }
    if(z < 11.0)
    {
        if(x < -1.25 || x > 1.25)
        {
            return false;
        }
    }
    return true;
}

enum Action
{
    Move_forward,
    Move_backward,
    Strafe_right,
    Strafe_left,
    Turn_right,
    Turn_left
};

const static struct Action_map
{
    int input;
    Action action;
} action_map[] =
{
    DIK_NUMPAD2, Move_backward,
    DIK_DOWN, Move_backward,
    DIK_NUMPAD8, Move_forward,
    DIK_UP, Move_forward,
    DIK_D, Strafe_right,
    DIK_A, Strafe_left,
    DIK_NUMPAD4, Turn_left,
    DIK_LEFT, Turn_left,
    DIK_NUMPAD6, Turn_right,
    DIK_RIGHT, Turn_right,
};

//---------------------------------------------------------------------------
// TODO: modularize and move out of main
Camera Input_device::get_input(const Camera& camera)
{
    static long msec = 0;
    long tick_count;

    float new_x, new_y, new_z;

    new_x = camera.m_x;
    new_y = camera.m_y;
    new_z = camera.m_z;

    Camera new_camera = camera;

    // TODO: get number of times from system queue
    // TODO: this is a bunch of crap
    tick_count = ::GetTickCount();
    if(msec == 0)
    {
        msec = tick_count;
    }
    else //if((tick_count - msec) > 16)
    {
        const auto ticks = tick_count - msec;

        char keybuffer[256];
        //for(int i = 0; i < tick_count - msec; i+= 16) 
        {
            WindowsCommon::dprintf("tick_count - msec: %d\r\n", tick_count - msec);

            if(m_device->GetDeviceState(256, (LPVOID)&keybuffer) != DI_OK)
            {
                // TODO11: bail out if acquire fails.
                m_device->Acquire();
                m_device->GetDeviceState(256, (LPVOID)&keybuffer);
            }

            // TODO: 2014: should this be an input map rather than an action map?
            std::list<Action> actions;
            for(int ix = 0; ix < ARRAYSIZE(action_map); ++ix)
            {
                if(keybuffer[action_map[ix].input])
                {
                    actions.push_back(action_map[ix].action);
                }
            }

            actions.sort();
            actions.unique();

            const auto walk_distance_per_tick = 0.045f;
            const auto walk_distance = walk_distance_per_tick * ticks;
            const auto BLAH = 0.0174f;
            const auto sine = sinf(camera.m_degrees * (BLAH)) * walk_distance;
            const auto cosine = cosf(camera.m_degrees * (BLAH)) * walk_distance;
            const auto keyboard_rotational_speed_per_tick = 0.5f;
            const auto rotation_degrees = keyboard_rotational_speed_per_tick * ticks;

            for(auto action = actions.cbegin(); action != actions.cend(); ++action)
            {
                switch(*action)
                {
                    case Move_forward:
                    {
                        new_x -= sine;
                        float temp = camera.m_x;
                        if(TestPolys(new_x, new_y, new_z))
                        {
                            new_camera.m_x = new_x;
                        }
                        new_z += cosine;
                        if(TestPolys(temp, new_y, new_z))
                        {
                            new_camera.m_z = new_z;
                        }
                        break;
                    }

                    case Move_backward:
                    {
                        new_x += sine;
                        float temp = camera.m_x;
                        if(TestPolys(new_x, new_y, new_z))
                        {
                            new_camera.m_x = new_x;
                        }
                        new_z -= cosine;
                        if(TestPolys(temp, new_y, new_z))
                        {
                            new_camera.m_z = new_z;
                        }
                        break;
                    }
                    case Strafe_right:
                    {
                        new_x -= cosine;
                        float temp = camera.m_x;
                        if(TestPolys(new_x, new_y, new_z))
                        {
                            new_camera.m_x = new_x;
                        }
                        new_z -= sine;
                        if(TestPolys(temp, new_y, new_z))
                        {
                            new_camera.m_z = new_z;
                        }
                        break;
                    }

                    case Strafe_left:
                    {
                        new_x += cosine;
                        float temp = camera.m_x;
                        if(TestPolys(new_x, new_y, new_z))
                        {
                            new_camera.m_x = new_x;
                        }
                        new_z += sine;
                        if(TestPolys(temp, new_y, new_z))
                        {
                            new_camera.m_z = new_z;
                        }
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
        }

        // TODO: send data to system queue instead of moving camera
        msec = tick_count;
    }

    return new_camera;
}

