#pragma once

#include <WindowsCommon/CheckHR.h>

namespace Direct3D9Common
{

class DX9_exception : public WindowsCommon::HRESULT_exception
{
public:
    DX9_exception(HRESULT hr, _In_z_ const char* file_name, int line) noexcept;
};

inline void check_dx9(HRESULT hr)
{
    if(FAILED(hr))
    {
        // TODO: The file/line info are just dummies now to keep this compiling.
        assert(false);
        throw DX9_exception(hr, __FILE__, __LINE__);
    }
}

} // namespace Direct3D9Common


