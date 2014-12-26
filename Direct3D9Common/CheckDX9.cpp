#include "PreCompile.h"
#include "CheckDX9.h"   // Pick up forward declarations to ensure correctness.

namespace Direct3D9Common
{

DX9_exception::DX9_exception(HRESULT hr) : HRESULT_exception(hr)
{
}

void DX9_exception::get_error_string(_Out_writes_z_(size) PTSTR error_string, size_t size) const
{
    PCTSTR const dx_message = DXGetErrorString(m_hr);
    if(CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, dx_message, -1, TEXT("Unknown"), -1) != CSTR_EQUAL)
    {
        StringCchPrintf(error_string, size, TEXT("Error: %08x: %s"), m_hr, dx_message);
    }
    else
    {
        assert(!"DX error string could not be found.  Was check_hr intended?");
        HRESULT_exception::get_error_string(error_string, size);
    }
}

// Reference used so that the DX call can be used inline in the function.
// ex: throw_dx9err_buffer(D3DXCompileShader(..., &error_buffer, ...), error_buffer).
// Without the reference, the undefined order of parameter evaluation could cause nullptr to be passed for the second error_buffer parameter.
void throw_dx9err_buffer(HRESULT hr, ATL::CComPtr<ID3DXBuffer>& error_buffer)
{
    if(FAILED(hr) && (nullptr != error_buffer))
    {
        // Copy to enforce null termination.
        char error_string[256];
        if(SUCCEEDED(StringCbCopyNA(error_string, sizeof(error_string), static_cast<char*>(error_buffer->GetBufferPointer()), error_buffer->GetBufferSize())))
        {
            // Output the string before exception is thrown, as DirectX is shut down during exception processing.
            // TODO: Use Tracing code instead of calling ODS.
            OutputDebugStringA(error_string);
        }
    }

    check_dx9(hr);
}

} // namespace Direct3D9Common

