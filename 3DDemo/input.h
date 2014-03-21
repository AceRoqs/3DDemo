#ifndef INPUT_H
#define INPUT_H

// TODO: Not sure how to make this OS independent yet.  Moving input functions to a class
// since there is state to maintain, and statics suck - 3/7/2014.
class Input_device
{
public:
    Input_device(_In_ HINSTANCE hInstance, _In_ HWND hwnd);
    ~Input_device();
    void get_input(float* camera_x, float* camera_y, float* camera_z, float* camera_degrees);

private:
    ATL::CComPtr<IDirectInputDevice8> m_device;
};

#endif

