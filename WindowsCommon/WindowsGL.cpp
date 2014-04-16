#include "PreCompile.h"
#include "WindowsGL.h"      // Pick up forward declarations to ensure correctness.

namespace WindowsCommon
{

LRESULT WindowGL_window_procedure::window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT return_value = 0;

    switch(message)
    {
        case WM_SIZE:
        {
            RECT client_rectangle;
            ::GetClientRect(window, &client_rectangle);

            ::glViewport(client_rectangle.left, client_rectangle.top, client_rectangle.right, client_rectangle.bottom);

            break;
        }

        case WM_ERASEBKGND:
        {
            break;
        }

        default:
        {
            return_value = DefWindowProc(window, message, w_param, l_param);
            break;
        }
    }

    return return_value;
}

}

