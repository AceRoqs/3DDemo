#include "PreCompile.h"
#include "input.h"          // Pick up forward declarations to ensure correctness.
#include "HRException.h"

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

//---------------------------------------------------------------------------
// TODO: modularize and move out of main
void Input_device::get_input(float* camera_x, float* camera_y, float* camera_z, float* camera_degrees)
{
    static long msec = 0;
    long tick_count;

    float new_x, new_y, new_z;

    new_x = *camera_x;
    new_y = *camera_y;
    new_z = *camera_z;

    // TODO: get number of times from system queue
    // TODO: this is a bunch of crap
    tick_count = ::GetTickCount();
    if(msec == 0)
    {
        msec = tick_count;
    }
    else //if((tick_count - msec) > 16)
    {
        char keybuffer[256];
        //for(int i = 0; i < tick_count - msec; i+= 16) 
        {
            char sz[100];
            sprintf_s(sz, 100, "tick_count - msec: %d\r\n", tick_count - msec);
            OutputDebugStringA(sz);
            //int d = (tick_count - msec) / 16;

            if(m_device->GetDeviceState(256, (LPVOID)&keybuffer) != DI_OK)
            {
                // TODO11: bail out if acquire fails.
                m_device->Acquire();
                m_device->GetDeviceState(256, (LPVOID)&keybuffer);
            }

            const double SPEED = 0.09;
            //const double SPEED = 0.15;
            const double BLAH = 0.0174;
            if(keybuffer[DIK_NUMPAD2] | keybuffer[DIK_DOWN])
            {
                new_x += float(SPEED * sin(*camera_degrees * (BLAH)));
                float temp = *camera_x;
                if(TestPolys(new_x, new_y, new_z))
                {
                    *camera_x = new_x;
                }
                new_z += float(-SPEED * cos(*camera_degrees * (BLAH)));
                if(TestPolys(temp, new_y, new_z))
                {
                    *camera_z = new_z;
                }
            }
            if(keybuffer[DIK_NUMPAD8] | keybuffer[DIK_UP])
            {
                new_x += float(-SPEED * sin(*camera_degrees * (BLAH)));
                float temp = *camera_x;
                if(TestPolys(new_x, new_y, new_z))
                {
                    *camera_x = new_x;
                }
                new_z += float(SPEED * cos(*camera_degrees * (BLAH)));
                if(TestPolys(temp, new_y, new_z))
                {
                    *camera_z = new_z;
                }
            }
            if(keybuffer[DIK_D])
            {
                new_x += float(-SPEED * cos(*camera_degrees * (BLAH)));
                float temp = *camera_x;
                if(TestPolys(new_x, new_y, new_z))
                {
                    *camera_x = new_x;
                }
                new_z += float(-SPEED * sin(*camera_degrees * (BLAH)));
                if(TestPolys(temp, new_y, new_z))
                {
                    *camera_z = new_z;
                }
            }
            if(keybuffer[DIK_A])
            {
                new_x += float(SPEED * cos(*camera_degrees * (BLAH)));
                float temp = *camera_x;
                if(TestPolys(new_x, new_y, new_z))
                {
                    *camera_x = new_x;
                }
                new_z += float(SPEED * sin(*camera_degrees * (BLAH)));
                if(TestPolys(temp, new_y, new_z))
                {
                    *camera_z = new_z;
                }
            }
            if(keybuffer[DIK_NUMPAD4] | keybuffer[DIK_LEFT])
            {
                *camera_degrees -= 3;
            }
            if(keybuffer[DIK_NUMPAD6] | keybuffer[DIK_RIGHT])
            {
                *camera_degrees += 3;
            }
        }

        // TODO: send data to system queue instead of moving camera
        msec = tick_count;
    }
}

