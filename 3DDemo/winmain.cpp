#include "PreCompile.h"
#include "app.h"
#include "HRException.h"

// The command line and environment can also be accessed
// via __argc, __targv, and _tenviron.
// http://msdn.microsoft.com/en-us/library/ccdc34y7.aspx
// http://support.microsoft.com/kb/126571

// Declspec SAL is used instead of attribute SAL, as the WinMain declaration
// in the system headers still uses declspec SAL.
int WINAPI _tWinMain(__in HINSTANCE instance,   // Handle to the program instance.
                     HINSTANCE,                 // hInstPrev - Unused in Win32.
                     __in PTSTR command_line,   // Command line.
                     int show_command)          // How the window is to be displayed.
{
    UNREFERENCED_PARAMETER(command_line);

    // Default the return code to 0, which is registered as ERRORLEVEL=0 in
    // a batch file.  This means running the app was successful.
    int return_code = 0;

    try
    {
        WindowsCommon::app_run(instance, show_command);
    }
    catch(const WindowsCommon::HRESULT_exception& ex)
    {
        // This is the last chance to display information to the user, so
        // do not localize the error string, as that can fail.  Also, do
        // not use types like ostringstream which dynamically allocate memory.
        TCHAR message[128];
        ex.get_error_string(message, _countof(message));
        MessageBox(nullptr, message, TEXT("Error"), MB_OK | MB_ICONERROR);

        // Set the ERRORLEVEL to 1, indicating an error.
        return_code = 1;
    }

    return return_code;
}

