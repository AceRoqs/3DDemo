#ifndef PRECOMPILE_H
#define PRECOMPILE_H

#include <cassert>

// C++ Standard Library.
#include <array>
#include <cstdint>
#include <exception>
#include <functional>
#include <string>
#include <vector>

// Windows API.
#ifdef WIN32

// Define to prevent using namespace ATL by default.
// http://msdn.microsoft.com/en-us/library/a477k694.aspx
#define _ATL_NO_AUTOMATIC_NAMESPACE

// CComPtr is the only ATL component that is used.  Do not accidentally link to the ATL libs.
#define _ATL_NO_DEFAULT_LIBS

// Prevent CComPtr from throwing an exception on operator*.
#define _ATL_NO_EXCEPTIONS

// For ATL::CComPtr.  CComPtr is used (as opposed to _com_ptr_t) because it doesn't throw exceptions (unlike _com_ptr_t,
// which throws exceptions not rooted in std::exception), it returns HRESULTs which integrates well with code that uses
// DirectX, and doesn't release it's interface pointer in operator&.
#include <atlbase.h>

// Defines to decrease build times:
// http://support.microsoft.com/kb/166474
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <strsafe.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#endif  // WIN32

#ifdef _MSC_VER
#include "PreCPP11.h"
#endif

#include <gl/GL.h>

#endif

