#ifndef __DEBUGGER_DAP_HELPERS_H__
#define __DEBUGGER_DAP_HELPERS_H__

//#include <iostream>
//#define WIN32_LEAN_AND_MEAN
//using namespace std;
#include "wx/string.h"

namespace dbg_DAP
{
    //#include <winsock2.h> // include ws2_32 in the linker settings
    //NOTE: Use this cmd to see what TCP sockets are being used: netstat -an | findstr "TCP"
    // Find an available socket port
    //-int Find_available_port(int start_port = 49152, int end_port = 65535); // (ph 26/01/18)
    int Find_available_port(int start_port = 49152, int end_port = 49999);
    bool FindPortArgFormat(const wxString& Executable, wxString& ArgFormat); // (ph 26/01/19)

    long long HexStrToBinary(const wxString& hexString);
};
#endif //__DEBUGGER_DAP_HELPERS_H__
