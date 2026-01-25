#ifndef __DAP_TERMINAL_PANEL_H__
#define __DAP_TERMINAL_PANEL_H__

// System and library includes
#include <wx/textctrl.h>

// CB includes

class DAPTerminalManager : public wxDialog
{
    public:
        DAPTerminalManager(wxWindow    *   parent,
                           wxWindowID      id     = wxID_ANY,
                           const wxString & title = wxEmptyString,
                           const wxPoint & pos    = wxDefaultPosition,
                           const wxSize  & size   = wxDefaultSize,
                           long            style  = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
                          );
        ~DAPTerminalManager();

        void AppendString(wxString msg);

    private:
        //Responders to standard wxWidgets Messages
        void OnUserInput(wxKeyEvent & ke);

    protected:
        wxTextCtrl * m_txtTerminalCtrl;
        DECLARE_EVENT_TABLE()
};

#endif // __DAP_TERMINAL_PANEL_H__
