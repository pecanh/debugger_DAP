/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef _DEBUGGER_DAP_DLG_PROJECTOPTIONS_H__
#define _DEBUGGER_DAP_DLG_PROJECTOPTIONS_H__

#include <wx/intl.h>
#include "configurationpanel.h"
#include <settings.h>

#include "plugin.h"

class cbProject;
class wxListBox;
class CodeBlocksEvent;

namespace dbg_DAP
{

class DebuggerOptionsProjectDlg : public cbConfigurationPanel
{
    public:
        DebuggerOptionsProjectDlg(wxWindow * parent, Debugger_DAP * debugger, cbProject * project);
        ~DebuggerOptionsProjectDlg() override;

        wxString GetTitle() const override
        {
            return "DAP Debugger";
        }
        wxString GetBitmapBaseName() const override
        {
            return "debugger";
        }
        void OnApply() override;
        void OnCancel() override {}

    protected:
        void OnTargetSel(wxCommandEvent & event);
        void OnAdd(wxCommandEvent & event);
        void OnEdit(wxCommandEvent & event);
        void OnDelete(wxCommandEvent & event);
        void OnUpdateUI(wxUpdateUIEvent & event);

    private:
        void OnBuildTargetRemoved(CodeBlocksEvent & event);
        void OnBuildTargetAdded(CodeBlocksEvent & event);
        void OnBuildTargetRenamed(CodeBlocksEvent & event);
        void LoadCurrentRemoteDebuggingRecord();
        void SaveCurrentRemoteDebuggingRecord();

        Debugger_DAP * m_pDebuggerDAPMI;
        cbProject * m_pProject;
        wxArrayString m_OldPaths;
        int m_LastTargetSel;
        DECLARE_EVENT_TABLE()
};
}; // namespace dbg_DAP

#endif // _DEBUGGER_DAP_DLG_PROJECTOPTIONS_H__
