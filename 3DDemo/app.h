#ifndef APP_H
#define APP_H

namespace WindowsCommon{

class Frame_app
{
public:
    static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

    void app_run(HINSTANCE instance, int show_command);
};

}


#endif

