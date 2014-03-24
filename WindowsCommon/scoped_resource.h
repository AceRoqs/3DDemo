// N3830 - Scoped Resource - Generic RAII Wrapper for the Standard Library.
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3830.pdf
// This is a partial implementation of N3830, implemented for Visual C++ 2010, which is not C++14 ready.
// Implemented by Toby Jones.
// TODO: This is not Windows specific.  It should be in a more general library than WindowsCommon.

#ifndef STD_OPT_SCOPED_RESOURCE_H
#define STD_OPT_SCOPED_RESOURCE_H

#include <utility>

namespace std_opt
{

#define NOEXCEPT
#define EQUALS_DELETE

enum invoke_it
{
    once,
    again
};

template <typename DELETER, typename RES>
class scoped_resource
{
    DELETER m_deleter;
    RES m_resource;
    bool m_execute_on_destruction;

    scoped_resource& operator=(scoped_resource const&) EQUALS_DELETE;
    scoped_resource(scoped_resource const&) EQUALS_DELETE;

public:
    explicit scoped_resource(DELETER&& deleter, RES&& resource, bool should_run = true) NOEXCEPT : m_deleter(std::move(deleter)), m_resource(std::move(resource)), m_execute_on_destruction(should_run)
    {
    }

    explicit scoped_resource(const DELETER& deleter, const RES& resource, bool should_run = true) NOEXCEPT : m_deleter(deleter), m_resource(resource), m_execute_on_destruction(should_run)
    {
    }


    scoped_resource(scoped_resource&& other) NOEXCEPT : m_deleter(other.m_deleter), m_resource(std::move(other.m_resource)), m_execute_on_destruction(other.m_execute_on_destruction)
    {
        other.m_execute_on_destruction = false;
    }

    scoped_resource& operator=(scoped_resource&& other) NOEXCEPT
    {
        // TODO: If m_resource was movable, what is the point of scoped_resource?
        invoke();
        m_resource = std::move(other.m_resource);
        other.m_execute_on_destruction = false;
    }

    ~scoped_resource()
    {
        if(m_execute_on_destruction)
        {
            m_deleter(m_resource);
        }
    }

    void invoke(const invoke_it strategy = std_opt::once) NOEXCEPT
    {
        if (execute_on_destruction)
        {
            m_deleter(m_resource);
        }
        execute_on_destruction = (strategy == invoke_it::again);
    }

    // Note: Difference with N3830, this does not return a tuple.
    RES release() NOEXCEPT
    {
        m_execute_on_destruction = false;
        return std::move(m_resource);
    }

    // Note: Difference with N3830, this does not take a tuple.
    void reset(RES resource) NOEXCEPT
    {
        invoke(invoke_it::again);
        m_resource = std::move(resource);
    }

    // Note: Difference with N3830, operator RES not implemented.

    // Note: Difference with N3830, this does not return a tuple.
    RES get() const NOEXCEPT
    {
        return m_resource;
    }

    // Note: Difference with N3830, operator* not implemented.
    // Note: Difference with N3830, operator-> not implemented.

    const DELETER& get_deleter() const NOEXCEPT
    {
        return m_deleter;
    }
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

