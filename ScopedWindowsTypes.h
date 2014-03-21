#ifndef SCOPEDWINDOWSTYPES_H
#define SCOPEDWINDOWSTYPES_H

#include "UniqueResource.h"

namespace WindowsCommon
{

typedef WindowsCommon::Unique_resource<ATOM, std::function<void (ATOM)>> Scoped_atom;

}

#endif

