#ifndef VIDEO_H
#define VIDEO_H

#include "WGLState.h"
#include "app.h"

WindowsCommon::WGL_state Startup_Video(HINSTANCE hInstance, bool fWindowed, WindowsCommon::Frame_app* frame);
void Shutdown_Video(bool fWindowed);

#endif

