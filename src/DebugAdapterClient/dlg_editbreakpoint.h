/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef EDITBREAKPOINTDLG_H_INCLUDED
#define EDITBREAKPOINTDLG_H_INCLUDED


#include "scrollingdialog.h"
#include "definitions.h"

namespace dbg_DAP //(ph 2024/06/10)
{

class EditBreakpointDlg : public wxScrollingDialog
{
    public:
        EditBreakpointDlg(const dbg_DAP::DAPBreakpoint &breakpoint, wxWindow* parent = 0);
        ~EditBreakpointDlg() override;

        const dbg_DAP::DAPBreakpoint& GetBreakpoint() const { return m_breakpoint; }
    protected:
        void OnUpdateUI(wxUpdateUIEvent& event);
        void EndModal(int retCode) override;

        dbg_DAP::DAPBreakpoint m_breakpoint;
    private:
        DECLARE_EVENT_TABLE()
};

}//end namespace dbg_DAP
#endif // EDITBREAKPOINT_H_INCLUDED
