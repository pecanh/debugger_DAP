// System and library includes
//#include <wx/artprov.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>
#include <wx/sizer.h>

// CB includes
#include <globals.h>
#include <logmanager.h>
#include <manager.h>

// DAP debugger includes
#include "DAP_Terminal_Panel.h"

////////////////////////////////////// DAPTerminalManager /////////////////////////////////////////////

BEGIN_EVENT_TABLE(DAPTerminalManager, wxDialog)
    EVT_CHAR(DAPTerminalManager::OnUserInput)
END_EVENT_TABLE()


DAPTerminalManager::DAPTerminalManager(wxWindow * parent,
                                       wxWindowID       id,
                                       const wxString & title,
                                       const wxPoint  & pos,
                                       const wxSize  &  size,
                                       long             style)
    : wxDialog(parent, id, title, pos, size, style)
{
    SetSizeHints(wxDefaultSize, wxDefaultSize);
    wxPanel * panel = new wxPanel(this, wxID_ANY);
    m_txtTerminalCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_WORDWRAP | wxTE_PROCESS_ENTER);
    // Set up the sizer for the panel
    wxBoxSizer * panelSizer  = new wxBoxSizer(wxVERTICAL);
    panelSizer->Add(m_txtTerminalCtrl, 1, wxEXPAND);
    panel->SetSizer(panelSizer);
    // Set up the sizer for the frame and resize the frame
    // according to its contents
    wxBoxSizer * topSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(panel, 1, wxEXPAND);
    SetSizerAndFit(topSizer);
    //Center();
}

DAPTerminalManager::~DAPTerminalManager()
{
}

void DAPTerminalManager::OnUserInput(wxKeyEvent & kEvent)
{
    m_txtTerminalCtrl->AppendText(kEvent.GetUnicodeKey());
}

void DAPTerminalManager::AppendString(wxString msg)
{
    m_txtTerminalCtrl->AppendText(msg);
}
