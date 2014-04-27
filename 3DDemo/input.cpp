#include "PreCompile.h"
#include "input.h"          // Pick up forward declarations to ensure correctness.
#include "HRException.h"

namespace WindowsCommon
{

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

}

