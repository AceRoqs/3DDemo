#include "PreCompile.h"
#include "app.h"
#include <WindowsCommon/CheckHR.h>
#include <WindowsCommon/DebuggerTracing.h>
#include <PortableRuntime/Tracing.h>

// The command line and environment can also be accessed
// via __argc, __targv, and _tenviron.
// http://msdn.microsoft.com/en-us/library/ccdc34y7.aspx
// http://support.microsoft.com/kb/126571

int WINAPI _tWinMain(_In_ HINSTANCE instance,   // Handle to the program instance.
                     _In_opt_ HINSTANCE,        // hInstPrev - Unused in Win32.
                     _In_ PTSTR command_line,   // Command line.
                     _In_ int show_command)     // How the window is to be displayed.
{
    UNREFERENCED_PARAMETER(command_line);

    PortableRuntime::set_dprintf(WindowsCommon::debugger_dprintf);

    // Default the return code to 0, which is registered as ERRORLEVEL=0 in
    // a batch file.  This means running the app was successful.
    int return_code = 0;

    try
    {
        Demo::app_run(instance, show_command);
    }
    catch(const WindowsCommon::HRESULT_exception& ex)
    {
        // This is the last chance to display information to the user, so
        // do not localize the error string, as that can fail.  Also, do
        // not use types like ostringstream which dynamically allocate memory.
        TCHAR message[128];
        ex.get_error_string(&message[0], ARRAYSIZE(message));
        MessageBox(nullptr, &message[0], TEXT("Error"), MB_OK | MB_ICONERROR);

        // Set the ERRORLEVEL to 1, indicating an error.
        return_code = 1;
    }

    return return_code;
}

