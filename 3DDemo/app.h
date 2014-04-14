#ifndef APP_H
#define APP_H

namespace WindowsCommon
{

class Window_procedure
{
public:
    static LRESULT CALLBACK static_window_proc(__in HWND window, UINT message, WPARAM w_param, LPARAM l_param);
    virtual LRESULT window_proc(_In_ HWND window, UINT message, WPARAM w_param, LPARAM l_param) = 0;
};

void app_run(HINSTANCE instance, int show_command);

}

#endif

