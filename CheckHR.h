#ifndef CHECKHR_H
#define CHECKHR_H

namespace WindowsCommon
{

HRESULT hresult_from_last_error() NOEXCEPT;

class HRESULT_exception : public std::exception
{
public:
    HRESULT_exception(HRESULT hr);

    // Define a method besides exception::what() that doesn't require heap memory allocation.
    virtual void get_error_string(_Out_z_cap_(size) PTSTR error_string, size_t size) const;

    HRESULT m_hr;
};

inline void check_hr(HRESULT hr)
{
    assert(SUCCEEDED(hr));
    if(FAILED(hr))
    {
        assert(false);
        throw HRESULT_exception(hr);
    }
}

inline void check_windows_error(BOOL result)
{
    assert(result);
    if(!result)
    {
        HRESULT hr = hresult_from_last_error();
        assert(FAILED(hr));
        throw HRESULT_exception(hr);
    }
}

inline void check_with_custom_hr(BOOL result, HRESULT hr)
{
    assert(result);
    if(!result)
    {
        assert(FAILED(hr));
        throw HRESULT_exception(hr);
    }
}

// These macros should only be used to work around static analysis warnings.
#define CHECK_WINDOWS_ERROR(expr) WindowsCommon::check_windows_error(expr); __analysis_assume(expr);
#define CHECK_WITH_CUSTOM_HR(expr, hr) WindowsCommon::check_with_custom_hr(expr, hr); __analysis_assume(expr);

} // namespace WindowsCommon

#endif

