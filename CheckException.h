#pragma once

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

// This macro should only be used to work around static analysis warnings.
// TODO: passing a function as expr is not valid.  Check this.
#define CHECK_EXCEPTION(expr) { PortableRuntime::check_exception(expr); __analysis_assume(expr); }

}

