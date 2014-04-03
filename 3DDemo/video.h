#ifndef VIDEO_H
#define VIDEO_H

#include "WGLState.h"

WindowsCommon::WGL_state Startup_Video(HINSTANCE hInstance, bool fWindowed);
void Shutdown_Video(bool fWindowed);

#endif

