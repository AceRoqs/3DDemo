#ifndef VIDEO_H
#define VIDEO_H

#include "WGLState.h"

WindowsCommon::WGL_state Startup_Video(HINSTANCE hInstance, bool fWindowed, HDC* phdc);
void Shutdown_Video(bool fWindowed, HWND hwnd, HDC hdc);

#endif

