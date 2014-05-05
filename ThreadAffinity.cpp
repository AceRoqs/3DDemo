#include "PreCompile.h"
#include "ThreadAffinity.h"
#include "HRException.h"

namespace WindowsCommon
{

void lock_thread_to_first_processor()
{
    const HANDLE process = GetCurrentProcess();

    DWORD_PTR process_mask;
    DWORD_PTR system_mask;
    if(!GetProcessAffinityMask(process, &process_mask, &system_mask))
    {
        WindowsCommon::throw_hr(WindowsCommon::hresult_from_last_error());
    }

    if(process_mask != 0 && system_mask != 0)
    {
        const DWORD_PTR thread_mask = process_mask & ~(process_mask - 1);

        const HANDLE thread = GetCurrentThread();
        if(!SetThreadAffinityMask(thread, thread_mask))
        {
            WindowsCommon::throw_hr(WindowsCommon::hresult_from_last_error());
        }
    }
}

}
