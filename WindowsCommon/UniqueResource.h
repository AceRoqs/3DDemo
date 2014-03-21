#ifndef UNIQUERESOURCE_H
#define UNIQUERESOURCE_H

// TODO: This is not Windows specific.  It should be in a more general library.
namespace WindowsCommon
{

template<typename Ty, typename Deleter>
class Unique_resource
{
    // Not implemented to prevent accidental copying.
    Unique_resource(const Unique_resource&) EQUALS_DELETE;
    Unique_resource& operator=(const Unique_resource&) EQUALS_DELETE;

    Ty m_value;
    Deleter m_deleter;

public:
    // Deleter is not allowed to throw.
    explicit Unique_resource(const Ty value, const Deleter& deleter) : m_value(value), m_deleter(deleter)
    {
    }

    Unique_resource(Unique_resource&& resource) : m_value(resource)
    {
        resource.m_value = 0;
        m_deleter = resource.m_deleter;
    }

    ~Unique_resource()
    {
        if(m_value != 0)
        {
            m_deleter(m_value);
        }
    }

    operator Ty() const
    {
        return m_value;
    }

    Ty detach()
    {
        Ty value = m_value;
        m_value = 0;

        return value;
    }
};

} // namespace WindowsCommon

#endif

