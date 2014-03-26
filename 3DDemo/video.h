#ifndef VIDEO_H
#define VIDEO_H

#include "ScopedWindowsTypes.h"

WindowsCommon::Scoped_atom Startup_Video(HINSTANCE hInstance, bool fWindowed, HWND* phwnd, HDC* phdc);
void Shutdown_Video(bool fWindowed, HWND hwnd, HDC hdc);

#endif

