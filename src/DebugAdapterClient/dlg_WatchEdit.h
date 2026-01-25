/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef __DEBUGGER_DAP_DLG_WATCHEDIT_H__
#define __DEBUGGER_DAP_DLG_WATCHEDIT_H__

#include "scrollingdialog.h"

#include "definitions.h"

namespace dbg_DAP
{
class EditWatchDlg : public wxScrollingDialog
{
    public:
        EditWatchDlg(cb::shared_ptr<dbg_DAP::DAPWatch> watch, wxWindow * parent);
        ~EditWatchDlg() override;

    protected:
        void EndModal(int retCode) override;

        cb::shared_ptr<dbg_DAP::DAPWatch> m_watch;
};
}; // namespace dbg_DAP

#endif // __DEBUGGER_DAP_DLG_WATCHEDIT_H__
