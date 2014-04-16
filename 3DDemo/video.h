#ifndef VIDEO_H
#define VIDEO_H

#include "WindowsGL.h"

WindowsCommon::WGL_state Startup_OpenGL(HINSTANCE hInstance, bool fWindowed, WindowsCommon::Window_procedure* window_proc);
void Shutdown_OpenGL(bool fWindowed);

#endif

