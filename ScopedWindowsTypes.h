#ifndef SCOPEDWINDOWSTYPES_H
#define SCOPEDWINDOWSTYPES_H

#include <functional>

namespace WindowsCommon
{

typedef std_opt::unique_resource_t<ATOM, std::function<void (ATOM)>> Scoped_atom;
typedef std_opt::unique_resource_t<HWND, std::function<void (HWND)>> Scoped_window;

Scoped_atom make_scoped_window_class(_In_ ATOM atom, _In_ HINSTANCE instance);
Scoped_window make_scoped_window(_In_ HWND window);

template <typename RESOURCE, typename DELETER=std::function<void (RESOURCE)>>
class Scoped_resource
{
    DELETER m_deleter;
    RESOURCE m_resource;

    // Prevent copy.
    Scoped_resource& operator=(const Scoped_resource&) EQUALS_DELETE;
    Scoped_resource(const Scoped_resource&) EQUALS_DELETE;

public:
    Scoped_resource() : m_resource(0) {}
    ~Scoped_resource()
    {
        invoke();
    }

    Scoped_resource(RESOURCE resource, DELETER&& deleter) : m_deleter(std::move(deleter)), m_resource(resource) {}

    Scoped_resource(Scoped_resource&& other) NOEXCEPT :
        m_deleter(std::move(other.m_deleter)),
        m_resource(std::move(other.m_resource))
    {
        other.release();
    }

    Scoped_resource& operator=(Scoped_resource&& other) NOEXCEPT
    {
        // Handle A=A case.
        if(this != &other)
        {
            invoke();
            m_deleter = std::move(other.m_deleter);
            m_resource = std::move(other.m_resource);
            other.release();
        }

        return *this;
    }

    void invoke() NOEXCEPT
    {
        if(m_resource != 0)
        {
            m_deleter(m_resource);
            m_resource = 0;
        }
    }

    RESOURCE release() NOEXCEPT
    {
        RESOURCE resource = m_resource;
        m_resource = 0;

        return resource;
    }
};

Scoped_resource<ATOM> make_scoped_window_class2(_In_ ATOM atom, _In_ HINSTANCE instance);

}

#endif

