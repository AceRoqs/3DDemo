#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

namespace WindowsCommon
{

#define KEYBOARD_BUFFER_SIZE 256
typedef std::array<uint8_t, KEYBOARD_BUFFER_SIZE> Keyboard_state;

class Input_device
{
public:
    Input_device(_In_ HINSTANCE hInstance, _In_ HWND hwnd);
    ~Input_device();
    void get_input(_In_ Keyboard_state* keyboard_state);

private:
    ATL::CComPtr<IDirectInputDevice8> m_device;
};

}

#endif

