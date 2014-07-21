#ifndef CHECKEXCEPTION_H
#define CHECKEXCEPTION_H

namespace PortableRuntime
{

inline void check_exception(bool result)
{
    if(!result)
    {
        assert(false);
        throw std::exception();
    }
}

}

#endif

