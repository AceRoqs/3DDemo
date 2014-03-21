#ifndef VIDEO_H
#define VIDEO_H

bool Startup_Video(HINSTANCE hInstance, int iCmdShow, bool fWindowed, HWND* phwnd, HDC* phdc);
void Shutdown_Video(bool fWindowed, HWND hwnd, HDC hdc);

#endif

