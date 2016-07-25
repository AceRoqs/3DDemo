#pragma once

// Prevent <Windows.h> from defining min/max which is different from <algorithm>.
// Must be included before headers that include WinDef.h.
// http://support.microsoft.com/kb/143208
#define NOMINMAX

#include <cassert>

#ifdef _MSC_VER

// Microsoft CRT API.
// tchar.h must be included before strsafe.h.
#include <tchar.h>

#endif // _MSC_VER

// C++ Standard Library.
#define _USE_MATH_DEFINES   // M_PI definition.
#include <array>
#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <functional>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

// Windows API.
#ifdef _WIN32

// Defines to decrease build times:
// http://support.microsoft.com/kb/166474
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <wrl.h>

#endif  // _WIN32

#include <gl/GL.h>

