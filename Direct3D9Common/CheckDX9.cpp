#include "PreCompile.h"
#include "CheckDX9.h"   // Pick up forward declarations to ensure correctness.
#include <PortableRuntime/Tracing.h>
#include <PortableRuntime/Unicode.h>

namespace Direct3D9Common
{

_Use_decl_annotations_
DX9_exception::DX9_exception(HRESULT hr, const char* file_name, int line) : HRESULT_exception(hr, file_name, line)
{
    try
    {
        WCHAR wide_error_string[1024];
        PCWSTR dx_message = DXGetErrorStringW(m_hr);
        if(CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, dx_message, -1, TEXT("Unknown"), -1) == CSTR_EQUAL)
        {
            assert(!"DX error string could not be found.  Was check_hr intended?");
            dx_message = L"Unknown error.";
        }

        StringCchPrintfW(wide_error_string, ARRAYSIZE(wide_error_string), L"Error: %08x: %s", m_hr, dx_message);

        const auto error_string = PortableRuntime::utf8_from_utf16(wide_error_string);
        PortableRuntime::dprintf("!%s(%d): %s\n", file_name, line, error_string.c_str());

        // Just save off the message now, but do the full formatting in what(), to allow exception unwind to free up some resources.
        m_what = std::make_shared<std::string>(std::move(error_string));
    }
    catch(const std::bad_alloc& ex)
    {
        (void)(ex);     // Unreferenced parameter.

        PortableRuntime::dprintf("!Failed creation of exception object.\n");
    }
}

// Reference used so that the DX call can be used inline in the function.
// ex: throw_dx9err_buffer(D3DXCompileShader(..., &error_buffer, ...), error_buffer).
// Without the reference, the undefined order of parameter evaluation could cause nullptr to be passed for the second error_buffer parameter.
void throw_dx9err_buffer(HRESULT hr, Microsoft::WRL::ComPtr<ID3DXBuffer>& error_buffer)
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

