// N3949 - Scoped Resource - Generic RAII Wrapper for the Standard Library.
// http://isocpp.org/files/papers/N3949.pdf
// This is a partial implementation of N3949, implemented for Visual C++ 2010, which is not C++14 ready.
// Implemented by Toby Jones.
// TODO: This is not Windows specific.  It should be in a more general library than WindowsCommon.

#ifndef STD_OPT_UNIQUE_RESOURCE_H
#define STD_OPT_UNIQUE_RESOURCE_H

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

template <typename RESOURCE, typename DELETER>
class unique_resource_t
{
    RESOURCE m_resource;
    DELETER m_deleter;
    bool m_execute_on_destruction;

    unique_resource_t& operator=(const unique_resource_t&) EQUALS_DELETE;
    unique_resource_t(const unique_resource_t&) EQUALS_DELETE;

public:
    explicit unique_resource_t(RESOURCE&& resource, DELETER&& deleter, bool should_run = true) NOEXCEPT :
        m_resource(std::move(resource)),
        m_deleter(std::move(deleter)),
        m_execute_on_destruction(should_run)
    {
    }

    unique_resource_t(unique_resource_t&& other) NOEXCEPT :
        m_resource(std::move(other.m_resource)),
        m_deleter(other.m_deleter),
        m_execute_on_destruction(other.m_execute_on_destruction)
    {
        other.release();
    }

    unique_resource_t& operator=(unique_resource_t&& other) NOEXCEPT
    {
        // Note: Difference from N3949 reference implementation, this handles A=A case.
        if(this != &other)
        {
            // Use std::move on m_resource, even though it is most likely is not movable if used with unique_resource_t.
            // If it is movable, then std::move still provides the proper semantics.
            invoke();
            m_resource = std::move(other.m_resource);
            m_deleter = std::move(other.m_deleter);
            m_execute_on_destruction = other.m_execute_on_destruction;
            other.release();
        }

        return *this;
    }

    ~unique_resource_t()
    {
        invoke();
    }

    void invoke(const invoke_it strategy = std_opt::once) NOEXCEPT
    {
        if(m_execute_on_destruction)
        {
            // Note: Difference from N3949, there is no try/catch block here.  Deleter should be noexcept.
            m_deleter(m_resource);
        }
        m_execute_on_destruction = (strategy == std_opt::again);
    }

    RESOURCE&& release() NOEXCEPT
    {
        m_execute_on_destruction = false;
        return std::move(m_resource);
    }

    void reset(RESOURCE&& resource) NOEXCEPT
    {
        // Note: Difference from N3949, operator== is required for resource, and this detects this->reset(this->m_resource).
        assert(!m_execute_on_destruction || resource != m_resource);

        invoke(std_opt::again);
        m_resource = std::move(resource);
    }

    operator const RESOURCE&() const NOEXCEPT
    {
        return m_resource;
    }

    const RESOURCE& get() const NOEXCEPT
    {
        return m_resource;
    }

    // Note: Difference from N3949, operator* not implemented.
    // Note: Difference from N3949, operator-> not implemented.

    const DELETER& get_deleter() const NOEXCEPT
    {
        return m_deleter;
    }
};

template<typename RESOURCE, typename DELETER>
unique_resource_t<RESOURCE, DELETER> unique_resource(RESOURCE&& resource, DELETER deleter)
{
    return unique_resource_t<RESOURCE, DELETER>(std::move(resource), std::move(deleter));
}
template<typename RESOURCE, typename DELETER>
unique_resource_t<RESOURCE, DELETER> unique_resource_checked(RESOURCE resource, RESOURCE invalid, DELETER deleter)
{
    auto should_run = (resource != invalid);
    return unique_resource_t<RESOURCE, DELETER>(std::move(resource), std::move(deleter), should_run);
}

}

#endif

