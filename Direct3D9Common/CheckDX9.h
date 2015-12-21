#pragma once

#include <WindowsCommon/CheckHR.h>

namespace Direct3D9Common
{

class DX9_exception : public WindowsCommon::HRESULT_exception
{
public:
    DX9_exception(HRESULT hr);

    virtual void get_error_string(_Out_writes_z_(size) PTSTR error_string, size_t size) const NOEXCEPT OVERRIDE;
};

inline void check_dx9(HRESULT hr)
{
    if(FAILED(hr))
    {
        assert(false);
        throw DX9_exception(hr);
    }
}

void throw_dx9err_buffer(HRESULT hr, ATL::CComPtr<ID3DXBuffer>& error_buffer);

} // namespace Direct3D9Common


