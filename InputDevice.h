#pragma once

namespace WindowsCommon
{

// Code assumes that KEYBOARD_BUFFER_SIZE <= MAXDWORD.
#define KEYBOARD_BUFFER_SIZE 256
typedef std::array<uint8_t, KEYBOARD_BUFFER_SIZE> Keyboard_state;

class Input_device
{
public:
    Input_device(_In_ HINSTANCE instance, _In_ HWND hwnd);
    ~Input_device();
    void get_input(_Out_ Keyboard_state* keyboard_state) const;

private:
    ATL::CComPtr<IDirectInputDevice8> m_device;
};

}

