#ifndef STD_OPT_SCOPED_RESOURCE_H
#define STD_OPT_SCOPED_RESOURCE_H

// scoped_resource.h : Implementation of N3830 - Scoped Resource - Generic RAII Wrapper for the Standard Library.
// N3830 is a proposal for a generic RAII wrapper.
// This is a partial implementation for VC2010, which doesn't include variadic template support.
// Implemented by Toby Jones.
// TODO: This is not Windows specific.  It should be in a more general library than WindowsCommon.

#include <utility>

namespace std_opt
{

#define NOEXCEPT

template <typename DELETER, typename RES>
class scoped_resource
{
    DELETER m_deleter;
    RES m_resource;
    bool m_should_run;

public:
    //explicit scoped_resource(DELETER&& deleter, R&&... resource, bool shouldRun = true) noexcept;
    explicit scoped_resource(DELETER&& deleter, RES&& resource, bool should_run = true) NOEXCEPT : m_deleter(std::move(deleter)), m_resource(std::move(resource)), m_should_run(should_run)
    {
    }

    //explicit scoped_resource(const DELETER& deleter, const R&... resource, bool shouldRun = true) noexcept;
    //scoped_resource(scoped_resource &&other) noexcept;
    //scoped_resource& operator=(scoped_resource &&other) noexcept;
    ~scoped_resource()
    {
        if(m_should_run)
        {
            m_deleter(m_resource);
        }
    }
    //void invoke(invoke_it const strategy = invoke_it::once) noexcept;
    //see below release() noexcept;
    //void reset(R... newresource) noexcept;
    //operator see below () const noexcept;
    //template<size_t n = 0>
    //see below get() const noexcept;
    //see below operator*() const noexcept;
    //see below operator->() const noexcept;
    //const DELETER & get_deleter() const noexcept;

};

template<typename DELETER, typename RES>
scoped_resource<DELETER, RES> make_scoped_resource(DELETER deleter, RES resource)
{
    return scoped_resource<DELETER, RES>(std::move(deleter), std::move(resource));
}
template<typename DELETER, typename RES>
scoped_resource<DELETER, RES> make_scoped_resource_checked(DELETER deleter, RES resource, RES invalid)
{
    auto should_run = (resource != invalid);
    return scoped_resource<DELETER, RES>(std::move(deleter), std::move(resource), should_run);
}

}

#endif

