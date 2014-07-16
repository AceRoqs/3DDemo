#include "PreCompile.h"
#include "CheckHR.h"    // Pick up forward declarations to ensure correctness.

namespace WindowsCommon
{

HRESULT_exception::HRESULT_exception(HRESULT hr) : m_hr(hr)
{
}

void HRESULT_exception::get_error_string(_Out_z_cap_(size) PTSTR error_string, size_t size) const
{
#ifdef _D3D9_H_
    // D3D errors should use D3D9_exception.
    assert(HRESULT_FACILITY(m_hr) != _FACD3D);
#endif

    // FORMAT_MESSAGE_IGNORE_INSERTS is required as arbitrary system messages may contain inserts.
    TCHAR message[128];
    if(0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                          nullptr,
                          m_hr,
                          0,
                          message,
                          ARRAYSIZE(message),
                          nullptr))
    {
        StringCchCopy(message, ARRAYSIZE(message), TEXT("Unknown"));
    }

    StringCchPrintf(error_string, size, TEXT("Error: %08x: %s"), m_hr, message);
}

HRESULT hresult_from_last_error() NOEXCEPT
{
    DWORD error = GetLastError();
    HRESULT hr = HRESULT_FROM_WIN32(error);
    assert(FAILED(hr));

    return hr;
}

} // namespace WindowsCommon

