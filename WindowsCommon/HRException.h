#ifndef HREXCEPTION_H
#define HREXCEPTION_H

namespace WindowsCommon
{

class HRESULT_exception : public std::exception
{
public:
    HRESULT_exception(HRESULT hr);

    // Define a method besides exception::what() that doesn't require heap memory allocation.
    virtual void get_error_string(_Out_z_cap_(size) PTSTR error_string, size_t size) const;

    HRESULT m_hr;
};

inline void throw_hr(HRESULT hr)
{
    if(FAILED(hr))
    {
        assert(false);
        throw HRESULT_exception(hr);
    }
}

HRESULT hresult_from_last_error() NOEXCEPT;

} // namespace WindowsCommon

#endif

