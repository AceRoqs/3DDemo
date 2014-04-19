#ifndef INPUT_H
#define INPUT_H

// TODO: Not sure how to make this OS independent yet.  Moving input functions to a class
// since there is state to maintain, and statics suck - 3/7/2014.
class Input_device
{
public:
    Input_device(_In_ HINSTANCE hInstance, _In_ HWND hwnd);
    ~Input_device();
    struct Camera get_input(const struct Camera& camera);

private:
    ATL::CComPtr<IDirectInputDevice8> m_device;
};

#endif

