#ifndef WGLSTATE_H
#define WGLSTATE_H

#include "ScopedWindowsTypes.h"

namespace WindowsCommon
{

struct WGL_state
{
// http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008
#if _MSC_VER == 1600
    // Add default and move constructors as VS2010 doesn't default define a move constructor.
    WGL_state()
    {
    }

    WGL_state(WGL_state&& state) : atom(std::move(state.atom)), window(std::move(state.window))
    {
    }
#else
#error This compiler may autodefine the default move constructor.
#endif

    Scoped_atom atom;
    Scoped_window window;
};

}

#endif

