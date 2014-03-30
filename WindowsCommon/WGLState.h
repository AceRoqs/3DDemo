#ifndef WGLSTATE_H
#define WGLSTATE_H

#include "ScopedWindowsTypes.h"

namespace WindowsCommon
{

struct WGL_state
{
#if 0
    WGL_state(std_opt::unique_resource_t<ATOM, std::function<void (ATOM)>>&& atom) :
        m_atom(atom)
    {
    }

    std_opt::unique_resource_t<ATOM, std::function<void (ATOM)>> m_atom;
#else
    Scoped_resource<ATOM> atom;
#endif
};

}

#endif

