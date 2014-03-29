#ifndef SCOPEDWINDOWSTYPES_H
#define SCOPEDWINDOWSTYPES_H

namespace WindowsCommon
{

typedef std_opt::unique_resource_t<ATOM, std::function<void (ATOM)>> Scoped_atom;
typedef std_opt::unique_resource_t<HWND, std::function<void (HWND)>> Scoped_window;

Scoped_atom make_scoped_window_class(_In_ ATOM atom, _In_ HINSTANCE instance);
Scoped_window make_scoped_window(_In_ HWND window);

// Called Scoped_atom2, really is a scoped_class.
class Scoped_atom2
{
    HINSTANCE m_instance;
    ATOM m_atom;

    // Prevent copy.
    Scoped_atom2& operator=(const Scoped_atom2&) EQUALS_DELETE;
    Scoped_atom2(const Scoped_atom2&) EQUALS_DELETE;

public:
    Scoped_atom2() : m_instance(nullptr), m_atom(0) {}
    ~Scoped_atom2()
    {
        if(m_atom != 0)
        {
            UnregisterClass(MAKEINTATOM(m_atom), m_instance);
        }
    }

    Scoped_atom2(ATOM atom, HINSTANCE instance) : m_instance(instance), m_atom(atom) {}

    Scoped_atom2(Scoped_atom2&& other) NOEXCEPT :
        m_instance(std::move(other.m_instance)),
        m_atom(std::move(other.m_atom))
    {
        other.release();
    }

    Scoped_atom2& operator=(Scoped_atom2&& other) NOEXCEPT
    {
        // Handle A=A case.
        if(this != &other)
        {
            invoke();
            m_atom = std::move(other.m_atom);
            m_instance = std::move(other.m_instance);
            other.release();
        }

        return *this;
    }

    void invoke() NOEXCEPT
    {
        if(m_atom != 0)
        {
            UnregisterClass(MAKEINTATOM(m_atom), m_instance);
            m_atom = 0;
            m_instance = 0;
        }
    }

    ATOM release() NOEXCEPT
    {
        ATOM atom = m_atom;
        m_atom = 0;
        m_instance = 0;

        return atom;
    }
};

Scoped_atom2 make_scoped_window_class2(_In_ ATOM atom, _In_ HINSTANCE instance);

}

#endif

