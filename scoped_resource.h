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

#ifndef NOEXCEPT
#if defined(_MSC_VER) && !defined(NDEBUG)
// MSVC + Debug build: use non-standard empty throw exception specification.
#define NOEXCEPT throw()
#else
#define NOEXCEPT
#endif
#endif

#ifndef EQUALS_DELETE
#define EQUALS_DELETE
#endif

enum invoke_it
{
    once,
    again
};

template <typename DELETER, typename RESOURCE>
class scoped_resource
{
    DELETER m_deleter;
    RESOURCE m_resource;
    bool m_execute_on_destruction;

    scoped_resource& operator=(const scoped_resource&) EQUALS_DELETE;
    scoped_resource(const scoped_resource&) EQUALS_DELETE;

public:
    explicit scoped_resource(DELETER&& deleter, RESOURCE&& resource, bool should_run = true) NOEXCEPT :
        m_deleter(std::move(deleter)),
        m_resource(std::move(resource)),
        m_execute_on_destruction(should_run)
    {
    }

    explicit scoped_resource(const DELETER& deleter, const RESOURCE& resource, bool should_run = true) NOEXCEPT :
        m_deleter(deleter),
        m_resource(resource),
        m_execute_on_destruction(should_run)
    {
    }

    scoped_resource(scoped_resource&& other) NOEXCEPT :
        m_deleter(other.m_deleter),
        m_resource(std::move(other.m_resource)),
        m_execute_on_destruction(other.m_execute_on_destruction)
    {
        other.m_execute_on_destruction = false;
    }

    // Note: Difference with N3830 reference implementation, this is marked NOEXCEPT.
    scoped_resource& operator=(scoped_resource&& other) NOEXCEPT
    {
        // Note: Difference with N3830 reference implementation, this handles A=A case.
        if(this != &other)
        {
            // Use std::move on m_resource, even though it is most likely is not movable if used with scoped_resource.
            // If it is movable, then std::move still provides the proper semantics.
            invoke();
            m_resource = std::move(other.m_resource);
            m_deleter = std::move(other.m_deleter);
            m_execute_on_destruction = other.m_execute_on_destruction;
            other.m_execute_on_destruction = false;
        }

        return *this;
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
        if (m_execute_on_destruction)
        {
            m_deleter(m_resource);
        }
        m_execute_on_destruction = (strategy == std_opt::again);
    }

    // Note: Difference with N3830, this does not return a tuple.
    RESOURCE release() NOEXCEPT
    {
        m_execute_on_destruction = false;
        return std::move(m_resource);
    }

    // Note: Difference with N3830, this does not take a tuple.
    void reset(RESOURCE resource) NOEXCEPT
    {
        // Note: Difference with N3830, operator== is required for resource, and this detects this->reset(this->m_resource).
        assert(!m_execute_on_destruction || resource != m_resource);

        invoke(std_opt::again);
        m_resource = std::move(resource);
    }

    // Note: Difference with N3830, operator RESOURCE not implemented.

    // Note: Difference with N3830, this does not return a tuple.
    RESOURCE get() const NOEXCEPT
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

template<typename DELETER, typename RESOURCE>
scoped_resource<DELETER, RESOURCE> make_scoped_resource(DELETER deleter, RESOURCE resource)
{
    return scoped_resource<DELETER, RESOURCE>(std::move(deleter), std::move(resource));
}
template<typename DELETER, typename RESOURCE>
scoped_resource<DELETER, RESOURCE> make_scoped_resource_checked(DELETER deleter, RESOURCE resource, RESOURCE invalid)
{
    auto should_run = (resource != invalid);
    return scoped_resource<DELETER, RESOURCE>(std::move(deleter), std::move(resource), should_run);
}

}

#endif

