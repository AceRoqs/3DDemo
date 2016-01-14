#pragma once

#include <cassert>

// C++ Standard Library.
#include <exception>
#include <memory>

// Windows API.
#ifdef _WIN32

// Defines to decrease build times:
// http://support.microsoft.com/kb/166474
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <strsafe.h>
#include <d3dx9.h>
#include <DxErr.h>

#include <wrl.h>

#endif  // _WIN32

