//-#include <iostream>
//-#define WIN32_LEAN_AND_MEAN
//-using namespace std;

//NOTE: Use this cmd to see what TCP sockets are being used: netstat -an | findstr "TCP"

#include "wx/string.h"
#include "logmanager.h"
#include <wx/utils.h>  // wxExecute

#define WXF wxString::Format
namespace dbg_DAP
{
// ----------------------------------------------------------------------------
//-int Find_available_port(int start_port = 49152, int end_port = 65535) // (ph 26/01/18)
int Find_available_port(int start_port = 49152, int end_port = 49999)
// ----------------------------------------------------------------------------
{
    LogManager* pLogMgr = Manager::Get()->GetLogManager();
    #if defined(__WIN32)
        wxString command = "cmd /c netstat -ano -p TCP | findstr :49*";
    #else
         wxString command =  "sh -c \"ss -t -a -n | grep :49\"";
    #endif

    wxArrayString output, errors;
    wxExecute(command, output, errors, wxEXEC_SYNC);
    if (errors.GetCount())
    {
        for (wxString err : errors)
            pLogMgr->DebugLogError(WXF("%s:%s", __FUNCTION__, err));
    }

    if (not output.GetCount()) return -1;

    wxString portStr;
    for (int port = start_port; port < end_port; ++port)
    {
        portStr = WXF("%d", port);
        for(wxString result : output)
        {
            if (result.Contains(portStr))
            {
                portStr.Clear();
                break;
            }
        }
        if (not portStr.empty()) break; //found a usable port
    }//endfor port
    if (not portStr.empty())
    {
        return wxAtoi(portStr);
    }
    return -1;

    // No available port found
    return -1;
}
// ----------------------------------------------------------------------------
long long HexStrToBinary(const wxString& hexString) {
// ----------------------------------------------------------------------------
    // Remove the '0x' prefix if it exists
    wxString cleanHexString = hexString;
    if (cleanHexString.StartsWith("0x")) {
        cleanHexString = cleanHexString.Mid(2);
    }

    // Convert the hex string to an integer
    long long hexInt;
    cleanHexString.ToLongLong(&hexInt, 16);

    return hexInt;
}
// ----------------------------------------------------------------------------
bool FindPortArgFormat(const wxString& Executable, wxString& ArgFormat) // (ph 26/01/19)
// ----------------------------------------------------------------------------
{
    // This function executes the specified executable with --help, captures its output
    // using wxExecute with wxEXEC_SYNC, and searches for either port pattern in the combined
    // output. It sets ArgFormat to the first matching pattern found and returns true, or leaves
    // it empty and returns false if neither pattern exists. The search uses case-insensitive
    // matching via Lower() for reliable detection across different output formats.

    ArgFormat = wxString();

    // Execute the program with --help and capture output
    wxArrayString output, errors;
    long exitCode = wxExecute(wxString::Format(wxT("%s --help"), Executable),
                             output, errors,
                             wxEXEC_SYNC | wxEXEC_NODISABLE);

    if (exitCode != 0 || output.IsEmpty()) {
        return false;
    }

    // Scan ONLY stdout (output array) - ignore errors array completely
    wxString helpText;
    for (size_t i = 0; i < output.GetCount(); ++i) {
        helpText += output[i] + wxT("\n");
    }
    // Search for port patterns (case insensitive)
    static const wxString patterns[] = {
        wxT("--port <port>"),
        wxT("--connection listen://localhost:<port>")
    };

    for (size_t i = 0; i < WXSIZEOF(patterns); ++i) {
        if (helpText.Lower().Contains(patterns[i].Lower())) {
            ArgFormat = patterns[i];
            return true;
        }
    }

    return false;
}

}//end namespace dbg_DAP

