#ifndef INPUT_H
#define INPUT_H

// TODO: namespace.

#define KEYBOARD_BUFFER_SIZE 256
typedef std::array<uint8_t, KEYBOARD_BUFFER_SIZE> Keyboard_state;

std::list<enum Action> actions_from_keyboard_state(const Keyboard_state& keyboard_state);

// TODO: Not sure how to make this OS independent yet.  Moving input functions to a class
// since there is state to maintain, and statics suck - 3/7/2014.
class Input_device
{
public:
    Input_device(_In_ HINSTANCE hInstance, _In_ HWND hwnd);
    ~Input_device();
    void get_input(_In_ Keyboard_state* keyboard_state);

private:
    ATL::CComPtr<IDirectInputDevice8> m_device;
};

#endif

