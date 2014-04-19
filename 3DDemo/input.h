#ifndef INPUT_H
#define INPUT_H

#include "Camera.h"

// TODO: Not sure how to make this OS independent yet.  Moving input functions to a class
// since there is state to maintain, and statics suck - 3/7/2014.
class Input_device
{
public:
    Input_device(_In_ HINSTANCE hInstance, _In_ HWND hwnd);
    ~Input_device();
    Camera get_input(const Camera& camera);

private:
    ATL::CComPtr<IDirectInputDevice8> m_device;
};

#endif

