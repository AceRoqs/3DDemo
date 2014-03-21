#ifndef DX9EXCEPTION_H
#define DX9EXCEPTION_H

#include "HRException.h"

namespace Direct3D9Common
{

class DX9_exception : public WindowsCommon::HRESULT_exception
{
public:
    DX9_exception(HRESULT hr);

    virtual void get_error_string(_Out_z_cap_(size) PTSTR error_string, size_t size) const OVERRIDE;
};

inline void throw_dx9err(HRESULT hr)
{
    if(FAILED(hr))
    {
        assert(false);
        throw DX9_exception(hr);
    }
}

void throw_dx9err_buffer(HRESULT hr, ATL::CComPtr<ID3DXBuffer>& error_buffer);

} // namespace Direct3D9Common


#endif

