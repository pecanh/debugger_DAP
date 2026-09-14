/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
*/
#define DAP_DEBUG_ENABLE 1

// C++ include files
#include <algorithm>
#include <map> //(ph 2024/08/11)

#include <wx/utils.h>   //(ph 2024/08/24) FindWindowbyName()

// CB include files (not DAP)
#include "cbdebugger_interfaces.h"
#include "cbplugin.h"
#include "cbproject.h"
#include "compilerfactory.h"

// DAP include files
#include "DAP_Watches.h"
#include "DAP_CallStack.h" // (ph 25/04/06)

#include "dlg_SettingsOptions.h"
#include "debugger_logger.h"
#include "DAP_Debugger_State.h"
#include "dlg_WatchEdit.h"

//XML file root tag for data
static const char * XML_CFG_ROOT_TAG = "Debugger_layout_file";

// constructor
// ----------------------------------------------------------------------------
DBG_DAP_Watches::DBG_DAP_Watches(cbDebuggerPlugin * plugin, dbg_DAP::LogPaneLogger * logger, dap::Client * pDAPClient) :
// ----------------------------------------------------------------------------
    m_plugin(plugin),
    m_pProject(nullptr),
    m_pLogger(logger),
    m_pDAPClient(pDAPClient)
{
    m_pInforRegisters =  new DBG_DAP_InfoRegisters(m_plugin, m_pLogger, wxString());

}

// ----------------------------------------------------------------------------
DBG_DAP_Watches::~DBG_DAP_Watches()
// ----------------------------------------------------------------------------
{
    // destructor
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::OnAttachReal()
// ----------------------------------------------------------------------------
{
    /// never called
    Manager::Get()->GetLogManager()->DebugLog(wxString::Format("%s %d", __PRETTY_FUNCTION__, __LINE__));

}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::OnReleaseReal(bool appShutDown)
// ----------------------------------------------------------------------------
{
    // Do not log anything as we are closing
    DAPDebuggerResetData(dbg_DAP::ResetDataType::ResetData_All);
    delete m_pInforRegisters;
}

// ----------------------------------------------------------------------------
dbg_DAP::DebuggerConfiguration & DBG_DAP_Watches::GetActiveConfigEx()
// ----------------------------------------------------------------------------
{
    return static_cast<dbg_DAP::DebuggerConfiguration &>(m_plugin->GetActiveConfig());
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::UpdateDebugDialogs(bool bClearAllData)
// ----------------------------------------------------------------------------
{
    cbWatchesDlg * pDialogWatches = Manager::Get()->GetDebuggerManager()->GetWatchesDialog();

    if (pDialogWatches)
    {
        pDialogWatches->RefreshUI();
    }
}
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::SetProject(cbProject * pProject)
// ----------------------------------------------------------------------------
{
    m_pProject = pProject;
}

// "===================================================================================="
// " ____    ____     ___        _   _____    ____   _____       ___      __  _____     "
// " |  _ \  |  _ \   / _ \      | | | ____|  / ___| |_   _|     |_ _|    / / |  ___|   "
// " | |_) | | |_) | | | | |  _  | | |  _|   | |       | |        | |    / /  | |_      "
// " |  __/  |  _ <  | |_| | | |_| | | |___  | |___    | |        | |   / /   |  _|     "
// " |_|     |_| \_\  \___/   \___/  |_____|  \____|   |_|       |___| /_/    |_|       "
// "                                                                                    "
// "===================================================================================="

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::OnProjectOpened(CodeBlocksEvent & event)
// ----------------------------------------------------------------------------
{
    // allow others to catch this
    event.Skip();

    if (GetActiveConfigEx().GetFlag(dbg_DAP::DebuggerConfiguration::PersistDebugElements))
    {
        LoadStateFromFile(event.GetProject());
    }
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::CleanupWhenProjectClosed(cbProject* pProject)
// ----------------------------------------------------------------------------
{
    if (GetActiveConfigEx().GetFlag(dbg_DAP::DebuggerConfiguration::PersistDebugElements))
    {
        SaveStateToFile(pProject);
    }

    size_t knt = m_DAP_watches.size();
    // Iterate from bottom to top to avoid invalid watch accesses because
    // RemoveWatch() may or may not remove a watch.
    for (size_t ii = knt; ii-- >0; )
    {
        cb::shared_ptr<dbg_DAP::DAPWatch> watch = m_DAP_watches[ii];

        if (watch->GetProject() == pProject)
        {
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Remove watch for \"%s\"", watch->GetSymbol()), dbg_DAP::LogPaneLogger::LineType::Debug);
            cbWatchesDlg* dialog = Manager::Get()->GetDebuggerManager()->GetWatchesDialog();
            dialog->RemoveWatch(watch);  // This call removes the watch from the GUI and debugger
        }
    }
}

// "=============================================================================================="
// "    __        __          _            _                                                      "
// "    \ \      / /   __ _  | |_    ___  | |__     ___   ___                                     "
// "     \ \ /\ / /   / _` | | __|  / __| | '_ \   / _ \ / __|                                    "
// "      \ V  V /   | (_| | | |_  | (__  | | | | |  __/ \__ \                                    "
// "       \_/\_/     \__,_|  \__|  \___| |_| |_|  \___| |___/                                    "
// "                                                                                              "
// "=============================================================================================="

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::CreateStartWatches()
// ----------------------------------------------------------------------------
{
    if (m_DAP_watches.empty())
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("No watches"), dbg_DAP::LogPaneLogger::LineType::Debug);
    }

    for (dbg_DAP::DAPWatchesContainer::iterator it = m_DAP_watches.begin(); it != m_DAP_watches.end(); ++it)
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Watch clear for symbol %s", (*it)->GetSymbol()), dbg_DAP::LogPaneLogger::LineType::Debug);
        (*it)->Reset();
    }

    if (not m_DAP_watches.empty())
    {
        CodeBlocksEvent event(cbEVT_DEBUGGER_UPDATED);
        event.SetInt(int(cbDebuggerPlugin::DebugWindows::Watches));
        Manager::Get()->ProcessEvent(event);
    }
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::UpdateDAPWatches(int updateType)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("updating watches"), dbg_DAP::LogPaneLogger::LineType::Debug);
    CodeBlocksEvent event(cbEVT_DEBUGGER_UPDATED);
    event.SetInt(updateType);
    Manager::Get()->ProcessEvent(event);

    // Update the Disassembly window if shown on screen
    wxWindow* pDisassembly = wxFindWindowByName("Disassembly");
    if (pDisassembly and pDisassembly->IsShownOnScreen())
    {
        m_plugin->RequestUpdate(cbDebuggerPlugin::Disassembly);
    }

    // Update the Memory window if shown on screen //(ph 2024/10/05)
    wxWindow* pMemoryWin = wxFindWindowByName("Memory");
    if (pMemoryWin and pMemoryWin->IsShownOnScreen())
    {
        m_plugin->RequestUpdate(cbDebuggerPlugin::ExamineMemory);
    }
}

// ----------------------------------------------------------------------------
cb::shared_ptr<cbWatch> DBG_DAP_Watches::AddWatch(const wxString & symbol, cb_unused bool update)
// ----------------------------------------------------------------------------
{
    // Invoked from the cb debugger menu handler

    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Add watch for \"%s\"", symbol), dbg_DAP::LogPaneLogger::LineType::Debug);
    cb::shared_ptr<dbg_DAP::DAPWatch> watch(new dbg_DAP::DAPWatch(m_pProject, m_pLogger, symbol, false));
    // check scope variables for symbol
    for (const dap::Variable& var : m_stackdapvariables)
    {
        if (symbol.IsSameAs(var.name))
        {
          #ifdef DAP_DEBUG_ENABLE
            wxString value = var.value.empty() ? "\"\"" : var.value;
            wxString attributes = wxEmptyString;

            for (const auto& attrib : var.presentationHint.attributes)
            {
                attributes += " " + attrib;
            }

            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                     __LINE__,
                                     wxString::Format(_("Var: %s = %s , variablesReference: %d  Type: %s, Hint: kind: %s , attributes %s , visibility: %s "),
                                                      var.name,
                                                      value,
                                                      var.variablesReference,
                                                      var.type,
                                                      var.presentationHint.kind,
                                                      attributes,
                                                      var.presentationHint.visibility
                                                     ),
                                     dbg_DAP::LogPaneLogger::LineType::UserDisplay);
          #endif
            watch->SetValue(var.value);
            watch->SetDAPVariableReference(var.variablesReference);
            watch->SetType(var.type);

            if (var.variablesReference > 0)
            {
                watch->SetHasBeenExpanded(false);
                watch->SetRangeArray(0, var.variablesReference);
                cbWatch::AddChild(watch, cb::shared_ptr<cbWatch>(new dbg_DAP::DAPWatch(m_pProject, m_pLogger, "updating...", false)));
            }

            //Sanity check: don't add duplicate watches // (ph 25/04/08)
            for (dbg_DAP::DAPWatchesContainer::iterator it = m_DAP_watches.begin(); it != m_DAP_watches.end(); ++it)
            {
                bool same = (symbol == (*it)->GetSymbol());
                same &= (watch->GetProject() == (*it)->GetProject());
                wxString watchType; watch->GetType(watchType);
                wxString containerType;  (*it)->GetType(containerType);
                same &= (watchType == containerType);
                ;//?if (same) return nullptr;
            }

            break;
        }
    }

    m_DAP_watches.push_back(watch);
    return watch;
}

// ----------------------------------------------------------------------------
cb::shared_ptr<cbWatch> DBG_DAP_Watches::AddWatch(dbg_DAP::DAPWatch* watch, cb_unused bool update)
// ----------------------------------------------------------------------------
{
    // External call from cbPlugin.h interface or
    // LoadStateFromFile() load of the saved watches file.

    //Sanity check: don't add duplicate watch
    // There, usually, isn't any dups, but it occured once when loading the saved watches.
    for (dbg_DAP::DAPWatchesContainer::iterator it = m_DAP_watches.begin(); it != m_DAP_watches.end(); ++it)
    {
        bool same = (watch->GetSymbol() == (*it)->GetSymbol());
        same &= (watch->GetProject() == (*it)->GetProject());
        wxString watchType; watch->GetType(watchType);
        wxString containerType;  (*it)->GetType(containerType);
        same &= (watchType == containerType);
        //?if (same) return nullptr; // (ph 25/04/16)
        if (same)
        {
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Remove watch for \"%s\"", watch->GetSymbol()), dbg_DAP::LogPaneLogger::LineType::Debug);
            cbWatchesDlg* dialog = Manager::Get()->GetDebuggerManager()->GetWatchesDialog();
            dialog->RemoveWatch(cb::shared_ptr<cbWatch> (*it)); // This call removes the watch from the GUI and debugger
            break;
        }
    }

    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Add watch for \"%s\"", watch->GetSymbol()), dbg_DAP::LogPaneLogger::LineType::Debug);
    cb::shared_ptr<dbg_DAP::DAPWatch> w(watch);
    m_DAP_watches.push_back(w); // (ph 25/04/02)

    // Do not call if this call was from the cbPlugin interface // (ph 25/04/03)
    cbWatchesDlg* dialog = Manager::Get()->GetDebuggerManager()->GetWatchesDialog();
    dialog->AddWatch(w);   // This call adds the watch to the debugger and GUI
    ExpandWatch(w); // (ph 25/04/17) See if expand will work for updating?

    if (Debugger_State::IsRunning())
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, "Need to wire up watch.", dbg_DAP::LogPaneLogger::LineType::Error);
    }

    return w;
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::DeleteWatch(cb::shared_ptr<cbWatch> watch)
// ----------------------------------------------------------------------------
{
    // Invoked from cbPlugin interface
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, "", dbg_DAP::LogPaneLogger::LineType::Debug);

    cb::shared_ptr<cbWatch> root_watch = cbGetRootWatch(watch);
    dbg_DAP::DAPWatchesContainer::iterator it = std::find(m_DAP_watches.begin(), m_DAP_watches.end(), root_watch);

    if (it == m_DAP_watches.end())
    {
        return;
    }

    if (m_DAP_watches.size() == 1)
    {
        m_DAP_watches.clear();
    }
    else
    {
        m_DAP_watches.erase(it);
    }
}

// ----------------------------------------------------------------------------
bool DBG_DAP_Watches::HasWatch(cb::shared_ptr<cbWatch> watch)
// ----------------------------------------------------------------------------
{
    // Invoked from cbPlugin interface
    if (watch == m_WatchLocalsandArgs)
    {
        return true;
    }

    dbg_DAP::DAPWatchesContainer::iterator it = std::find(m_DAP_watches.begin(), m_DAP_watches.end(), watch);
    return it != m_DAP_watches.end();
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::ShowWatchProperties(cb::shared_ptr<cbWatch> watch)
// ----------------------------------------------------------------------------
{
    // not supported for child nodes or memory ranges!
    if (watch->GetParent() || IsMemoryRangeWatch(watch))
    {
        return;
    }

    cb::shared_ptr<dbg_DAP::DAPWatch> real_watch = cb::static_pointer_cast<dbg_DAP::DAPWatch>(watch);
    dbg_DAP::EditWatchDlg dlg(real_watch, nullptr);
    PlaceWindow(&dlg);

    if (dlg.ShowModal() == wxID_OK)
    {
        DoWatches();
    }
}

// ----------------------------------------------------------------------------
bool DBG_DAP_Watches::SetWatchValue(cb::shared_ptr<cbWatch> watch, const wxString & value)
// ----------------------------------------------------------------------------
{
    // originarily invoked from watches dlg
    /// Originates from watchesdlg, but is never called anymore
    // Neither GDB/CDB debuggerGDB plugin, nor debuggerGDB_MI plugin ever called this function
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Functionality is never called!"), dbg_DAP::LogPaneLogger::LineType::Error);

    if (!Debugger_State::IsStopped() || !Debugger_State::IsRunning())
    {
        return false;
    }

    cb::shared_ptr<cbWatch> root_watch = cbGetRootWatch(watch);
    dbg_DAP::DAPWatchesContainer::iterator it = std::find(m_DAP_watches.begin(), m_DAP_watches.end(), root_watch);

    if (it == m_DAP_watches.end())
    {
        return false;
    }

    cb::shared_ptr<dbg_DAP::DAPWatch> real_watch = cb::static_pointer_cast<dbg_DAP::DAPWatch>(watch);
    uint64_t variableReference = real_watch->GetDAPVariableReference();
    wxUnusedVar(variableReference);
    //    AddStringCommand("-var-assign " + real_watch->GetID() + " " + value);
    //    m_actions.Add(new dbg_DAP::DAPWatchSetValueAction(*it, static_cast<dbg_DAP::DAPWatch*>(watch), value, m_pLogger));
    //    dbg_DAP::Action * update_action = new dbg_DAP::DAPWatchesUpdateAction(m_watches, m_pLogger);
    //    update_action->SetWaitPrevious(true);
    //    m_actions.Add(update_action);
    return true;
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::ExpandWatch(cb::shared_ptr<cbWatch> watch)
// ----------------------------------------------------------------------------
{
    // Invoked from cbPlugin.h interface
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, "", dbg_DAP::LogPaneLogger::LineType::Debug);

    if ( (not Debugger_State::IsStopped()) || (not Debugger_State::IsRunning()) )
    {
        return;
    }

    cb::shared_ptr<cbWatch> root_watch = cbGetRootWatch(watch);
    dbg_DAP::DAPWatchesContainer::iterator it = std::find(m_DAP_watches.begin(), m_DAP_watches.end(), root_watch);

    if (it != m_DAP_watches.end())
    {
        cb::shared_ptr<dbg_DAP::DAPWatch> real_watch = cb::static_pointer_cast<dbg_DAP::DAPWatch>(watch);

        if (not real_watch->HasBeenExpanded())
        {
            real_watch->RemoveChildren();
            real_watch->SetDAPChildVariableRequestSequence(m_pDAPClient->GetChildrenVariables(real_watch->GetDAPVariableReference(), dap::EvaluateContext::VARIABLES, 0));
        }
    }
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::CollapseWatch(cb::shared_ptr<cbWatch> watch)
// ----------------------------------------------------------------------------
{
    // Invoked from cbPlugin interface

    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Functionality not completed yet!"), dbg_DAP::LogPaneLogger::LineType::Error);

    if (not Debugger_State::IsStopped() || !Debugger_State::IsRunning())
    {
        return;
    }

    cb::shared_ptr<cbWatch> root_watch = cbGetRootWatch(watch);
    dbg_DAP::DAPWatchesContainer::iterator it = std::find(m_DAP_watches.begin(), m_DAP_watches.end(), root_watch);

    if (it != m_DAP_watches.end())
    {
        cb::shared_ptr<dbg_DAP::DAPWatch> real_watch = cb::static_pointer_cast<dbg_DAP::DAPWatch>(watch);

        if (real_watch->HasBeenExpanded() && real_watch->DeleteOnCollapse())
        {
            // m_actions.Add(new dbg_DAP::DAPWatchCollapseAction(*it, real_watch, m_watches, m_pLogger));
            asm("nop"); /**Debugging**/
        }
    }
}
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::UpdateWatch(cb_unused cb::shared_ptr<cbWatch> watch, DBG_DAP_CallStack* pDAPCallStack)
// ----------------------------------------------------------------------------
{
    // Invoked from the cbPlugin interface or watchesdlg::OnMenuUpdate()

    //-Done- m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Functionality supported"), dbg_DAP::LogPaneLogger::LineType::Error);
    dbg_DAP::DAPWatchesContainer::iterator it = std::find(m_DAP_watches.begin(), m_DAP_watches.end(), watch);

    if (it == m_DAP_watches.end())
    {
        return;
    }

    if (Debugger_State::IsRunning())
    {
        //  For DAP, use VariablesRequest //(ph 2024/11/01)
        cb::shared_ptr<dbg_DAP::DAPWatch> real_watch = cb::static_pointer_cast<dbg_DAP::DAPWatch>(watch);
        //GetVariableInfo will issue the DAP variablesRequest to the Dap client
        m_pDAPClient->GetVariableInfo(real_watch->GetDAPChildVariableRequestSequence());
        EvaluateWatchWithCallback(real_watch, pDAPCallStack);
    }
}
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::EvaluateWatchWithCallback(cb::shared_ptr<dbg_DAP::DAPWatch> real_watch, DBG_DAP_CallStack* pDAPCallStack) // (ph 25/04/06)
// ----------------------------------------------------------------------------
{
    // Evaluate watch text

    int activeStackFrame = pDAPCallStack->GetActiveStackFrame();
    if (activeStackFrame < 0) return; //(ph 2025/01/26)

    cbStackFrame::ConstPointer pStackFrame = pDAPCallStack->GetStackFrame(activeStackFrame);
    const dbg_DAP::cbStackFrameXtn* pXtn = static_cast<const dbg_DAP::cbStackFrameXtn*>(pStackFrame.get());
    int stackID = 0;
    if (pXtn)
        stackID = pXtn->GetDAPStackId();
    wxString token = real_watch->GetSymbol();
    m_pDAPClient->EvaluateExpression(
        token, stackID, dap::EvaluateContext::WATCH,
        [this, token, real_watch](bool success, const wxString& result, const wxString& type, int variablesReference) {
            EvaluateWatchCallback(success, result, type, variablesReference, token, real_watch);
        }
    );
}
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::EvaluateWatchCallback(bool success, const wxString& result, const wxString& type, int variablesReference, const wxString& text, cb::shared_ptr<dbg_DAP::DAPWatch> real_watch)
// ----------------------------------------------------------------------------
{
    // Shows result of EvaluateWatchWithCallback() for watch text/token

    wxString output;
    if (!success) {
        output << "ERROR: failed to evaluate expression: `" << text << "`";
        // AddLog(output);
        Manager::Get()->GetLogManager()->DebugLogError(output);
        return;
    }
    output << text << " = " << result << " [" << type << "]. variablesReference: " << variablesReference;
    // AddLog(output);
    Manager::Get()->GetLogManager()->DebugLog(output);

    //-cb::shared_ptr<dbg_DAP::DAPWatch> pWatch(new dbg_DAP::DAPWatch(m_pProject, m_pLogger, text, true,true));
    real_watch->SetValue(result);
    real_watch->SetType(type);
    // Display watches. disassembly, memory
    UpdateDAPWatches(int(cbDebuggerPlugin::DebugWindows::Watches));
}

// ----------------------------------------------------------------------------
void DBG_DAP_Watches::DoWatches()
// ----------------------------------------------------------------------------
{
    if (not Debugger_State::IsRunning())
    {
        return;
    }

    dbg_DAP::DebuggerConfiguration & config = GetActiveConfigEx();
    bool bWatchFuncLocalsArgs = config.GetFlag(dbg_DAP::DebuggerConfiguration::WatchFuncLocalsArgs);

    if (bWatchFuncLocalsArgs)
    {
        if (m_WatchLocalsandArgs == nullptr)
        {
            m_WatchLocalsandArgs = cb::shared_ptr<dbg_DAP::DAPWatch>(new dbg_DAP::DAPWatch(m_pProject, m_pLogger, "Function locals and arguments", false));
            m_WatchLocalsandArgs->Expand(true);
            m_WatchLocalsandArgs->MarkAsChanged(false);
            cbWatchesDlg * watchesDialog = Manager::Get()->GetDebuggerManager()->GetWatchesDialog();
            watchesDialog->AddSpecialWatch(m_WatchLocalsandArgs, true);
        }
    }

    //- m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, "Need to wire up DoWatches.", dbg_DAP::LogPaneLogger::LineType::Error);
    // Update watches now
    CodeBlocksEvent event(cbEVT_DEBUGGER_UPDATED);
    event.SetInt(int(cbDebuggerPlugin::DebugWindows::Watches));
    Manager::Get()->ProcessEvent(event);
}

// "================================================================================================"
// "     __  __                                               ____                                  "
// "    |  \/  |   ___   _ __ ___     ___    _ __   _   _    |  _ \    __ _   _ __     __ _    ___  "
// "    | |\/| |  / _ \ | '_ ` _ \   / _ \  | '__| | | | |   | |_) |  / _` | | '_ \   / _` |  / _ \ "
// "    | |  | | |  __/ | | | | | | | (_) | | |    | |_| |   |  _ <  | (_| | | | | | | (_| | |  __/ "
// "    |_|  |_|  \___| |_| |_| |_|  \___/  |_|     \__, |   |_| \_\  \__,_| |_| |_|  \__, |  \___| "
// "                                                |___/                             |___/         "
// "================================================================================================"

// ----------------------------------------------------------------------------
cb::shared_ptr<cbWatch> DBG_DAP_Watches::AddMemoryRange(uint64_t llAddress, uint64_t llSize, const wxString & symbol, bool update)
// ----------------------------------------------------------------------------
{
    /// No calls or interface exists for AddMemoryRange in either cdb/gdb or debuggerMI //(ph 2024/11/02)
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Functionality not supported yet!"), dbg_DAP::LogPaneLogger::LineType::Error);
    cb::shared_ptr<dbg_DAP::DAPMemoryRangeWatch> watch(new dbg_DAP::DAPMemoryRangeWatch(m_pProject, m_pLogger, llAddress, llSize, symbol));
    //
    //    watch->SetSymbol(symbol);
    //    watch->SetAddress(llAddress);
    //
    //    m_memoryRanges.push_back(watch);
    //    m_mapWatchesToType[watch] = dbg_DAP::DAPWatchType::MemoryRange;
    //
    //    if (IsRunning())
    //    {
    //        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Adding watch for: address: %#018llx  size:%lld", llAddress, llSize), dbg_DAP::LogPaneLogger::LineType::Warning);
    //        m_actions.Add(new dbg_DAP::DAPMemoryRangeWatchCreateAction(watch, m_pLogger));
    //    }
    //
    return watch;
}


// ----------------------------------------------------------------------------
bool DBG_DAP_Watches::IsMemoryRangeWatch(const cb::shared_ptr<cbWatch> & watch)
// ----------------------------------------------------------------------------
{
    dbg_DAP::DAPMapWatchesToType::const_iterator it = m_mapWatchesToType.find(watch);

    if (it == m_mapWatchesToType.end())
    {
        return false;
    }
    else
    {
        return (it->second == dbg_DAP::DAPWatchType::MemoryRange);
    }
}

// "================================================================================================"
// "     __  __   ___   ____     ____                                                               "
// "    |  \/  | |_ _| / ___|   / ___|                                                              "
// "    | |\/| |  | |  \___ \  | |                                                                  "
// "    | |  | |  | |   ___) | | |___                                                               "
// "    |_|  |_| |___| |____/   \____|                                                              "
// "                                                                                                "
// "================================================================================================"

void DBG_DAP_Watches::DAPDebuggerResetData(dbg_DAP::ResetDataType bClearAllData)
{
}

// "================================================================================================"
// "         ____       _     __     __  _____     ____    _____      _      _____   _____          "
// "        / ___|     / \    \ \   / / | ____|   / ___|  |_   _|    / \    |_   _| | ____|         "
// "        \___ \    / _ \    \ \ / /  |  _|     \___ \    | |     / _ \     | |   |  _|           "
// "         ___) |  / ___ \    \ V /   | |___     ___) |   | |    / ___ \    | |   | |___          "
// "        |____/  /_/   \_\    \_/    |_____|   |____/    |_|   /_/   \_\   |_|   |_____|         "
// "                                                                                                "
// "================================================================================================"

// ----------------------------------------------------------------------------
bool DBG_DAP_Watches::SaveStateToFile(cbProject * pProject)
// ----------------------------------------------------------------------------
{
    //There are two types of state we should save:
    //1, breakpoints
    //2, watches
    //Create a file according to the m_pProject
    wxString projectFilename = pProject->GetFilename();

    if (projectFilename.IsEmpty())
    {
        return false;
    }

    //saved file name&extention
    wxFileName fname(projectFilename);
    fname.SetExt("bps");
    tinyxml2::XMLDocument doc;
    // doc.InsertEndChild(tinyxml2::XMLDeclaration("1.0", "UTF-8", "yes"));
    tinyxml2::XMLNode * rootnode = doc.InsertEndChild(doc.NewElement(XML_CFG_ROOT_TAG));

    if (!rootnode)
    {
        return false;
    }

    // ********************  Save debugger name ********************
    wxString compilerID = pProject->GetCompilerID();
    int compilerIdx = CompilerFactory::GetCompilerIndex(compilerID);
    Compiler * pCompiler = CompilerFactory::GetCompiler(compilerIdx);
    const CompilerPrograms & pCompilerProgsp = pCompiler->GetPrograms();
    tinyxml2::XMLNode * pCompilerNode = rootnode->InsertEndChild(doc.NewElement("CompilerInfo"));
    dbg_DAP::AddChildNode(pCompilerNode, "CompilerName", pCompiler->GetName());
    // dbg_DAP::AddChildNode(pCompilerNode, "C_Compiler", pCompilerProgsp.C);
    // dbg_DAP::AddChildNode(pCompilerNode, "CPP_Compiler",  pCompilerProgsp.CPP);
    // dbg_DAP::AddChildNode(pCompilerNode, "DynamicLinker_LD",  pCompilerProgsp.LD);
    // dbg_DAP::AddChildNode(pCompilerNode, "StaticLinker_LIB",  pCompilerProgsp.LIB);
    // dbg_DAP::AddChildNode(pCompilerNode, "Make",  pCompilerProgsp.MAKE);
    dbg_DAP::AddChildNode(pCompilerNode, "DBGconfig",  pCompilerProgsp.DBGconfig);

    // ********************  Save Watches ********************
    tinyxml2::XMLElement * pElementWatchesList = doc.NewElement("WatchesList");
    pElementWatchesList->SetAttribute("count", static_cast<int64_t>(m_DAP_watches.size()));
    tinyxml2::XMLNode * pWatchesMasterNode = rootnode->InsertEndChild(pElementWatchesList);

    for (dbg_DAP::DAPWatchesContainer::iterator it = m_DAP_watches.begin(); it != m_DAP_watches.end(); ++it)
    {
        dbg_DAP::DAPWatch & watch = **it;

        if (watch.GetProject() == pProject)
        {
            if (watch.ForTooltip()) continue; // don't save tooltip watches //(ph 2024/05/28)
            watch.SaveWatchToXML(pWatchesMasterNode);
        }
    }

    // ********************  Save Memory Range Watches ********************
    //    tinyxml2::XMLElement* pElementMemoryRangeList = doc.NewElement("MemoryRangeList");
    //    pElementMemoryRangeList->SetAttribute("count", m_memoryRanges.size());
    //    tinyxml2::XMLNode* pMemoryRangeMasterNode = rootnode->InsertEndChild(pElementMemoryRangeList);
    //
    //    for (dbg_DAP::DAPMemoryRangeWatchesContainer::iterator it = m_memoryRanges.begin(); it != m_memoryRanges.end(); ++it)
    //    {
    //        dbg_DAP::DAPMemoryRangeWatch& memoryRange = **it;
    //
    //        if (memoryRange.GetProject() == pProject)
    //        {
    //            memoryRange.SaveWatchToXML(pMemoryRangeMasterNode);
    //        }
    //    }
    // ********************  Save XML to disk ********************
    return doc.SaveFile(cbU2C(fname.GetFullPath()), false);
}

// ----------------------------------------------------------------------------
bool DBG_DAP_Watches::LoadStateFromFile(cbProject * pProject)
// ----------------------------------------------------------------------------
{
    if (not pProject) return false;

    wxString projectFilename = pProject->GetFilename();

    if (projectFilename.IsEmpty())
        return false;

    wxFileName fname(projectFilename);
    fname.SetExt("bps");

    if (!fname.FileExists())
    {
        return false;
    }

    //Open XML file
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError eResult = doc.LoadFile(cbU2C(fname.GetFullPath()));

    if (eResult != tinyxml2::XMLError::XML_SUCCESS)
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Could not open the file '\%s\" due to the error: %s"), fname.GetFullPath(), doc.ErrorIDToName(eResult)), dbg_DAP::LogPaneLogger::LineType::Error);
        return false;
    }

    tinyxml2::XMLElement * root = doc.FirstChildElement(XML_CFG_ROOT_TAG);

    if (!root)
    {
        return false;
    }

    // ******************** Load watches ********************
    tinyxml2::XMLElement * pElementWatchesList = root->FirstChildElement("WatchesList");

    if (pElementWatchesList)
    {
        for (tinyxml2::XMLElement * pWatchElement = pElementWatchesList->FirstChildElement("Watch");
                pWatchElement;
                pWatchElement = pWatchElement->NextSiblingElement())
        {
            wxString DAPWatchClassName = dbg_DAP::ReadChildNodewxString(pWatchElement, "DAPWatchClassName");

            if (DAPWatchClassName.IsSameAs("DAPWatch"))
            {
                dbg_DAP::DAPWatch * watch = new dbg_DAP::DAPWatch(pProject, m_pLogger, "", false);
                watch->LoadWatchFromXML(pWatchElement);

                if (not watch->GetSymbol().IsEmpty())
                {
                    // See debuggermenu.cpp DebuggerMenuHandler::OnAddWatch(...) function
                    // This call adds the watch to the debugger and GUI watch dialog also
                    cb::shared_ptr<cbWatch> watchAdded = AddWatch(watch, true);
                }
            }
        }
    }

    // ******************** Load Memory Range Watches ********************
    //    tinyxml2::XMLElement* pElementMemoryRangeList = root->FirstChildElement("MemoryRangeList");
    //    if (pElementMemoryRangeList)
    //    {
    //        for(    tinyxml2::XMLElement* pWatchElement = pElementMemoryRangeList->FirstChildElement("MemoryRangeWatch");
    //                pWatchElement;
    //                pWatchElement = pWatchElement->NextSiblingElement())
    //        {
    //            wxString DAPMemoryRangeWatchName = dbg_DAP::ReadChildNodewxString(pWatchElement, "DAPMemoryRangeWatch");
    //            if (DAPMemoryRangeWatchName.IsSameAs("DAPMemoryRangeWatch"))
    //            {
    //                dbg_DAP::DAPMemoryRangeWatch* memoryRangeWatch = new dbg_DAP::DAPMemoryRangeWatch(pProject, m_pLogger, 0, 0, wxEmptyString );
    //                memoryRangeWatch->LoadWatchFromXML(pWatchElement, this);
    //            }
    //        }
    //    }
    // ******************** Finished Load ********************
    return true;
}

// "======================================================================================================================="
// "                     ____       _      ____        _____  __     __  _____   _   _   _____   ____                      "
// "                    |  _ \     / \    |  _ \      | ____| \ \   / / | ____| | \ | | |_   _| / ___|                     "
// "                    | | | |   / _ \   | |_) |     |  _|    \ \ / /  |  _|   |  \| |   | |   \___ \                     "
// "                    | |_| |  / ___ \  |  __/      | |___    \ V /   | |___  | |\  |   | |    ___) |                    "
// "                    |____/  /_/   \_\ |_|         |_____|    \_/    |_____| |_| \_|   |_|   |____/                     "
// "                                                                                                                       "
// "     _____   _   _   _   _    ____   _____   ___    ___    _   _   ____      ____    _____      _      ____    _____   "
// "    |  ___| | | | | | \ | |  / ___| |_   _| |_ _|  / _ \  | \ | | / ___|    / ___|  |_   _|    / \    |  _ \  |_   _|  "
// "    | |_    | | | | |  \| | | |       | |    | |  | | | | |  \| | \___ \    \___ \    | |     / _ \   | |_) |   | |    "
// "    |  _|   | |_| | | |\  | | |___    | |    | |  | |_| | | |\  |  ___) |    ___) |   | |    / ___ \  |  _ <    | |    "
// "    |_|      \___/  |_| \_|  \____|   |_|   |___|  \___/  |_| \_| |____/    |____/    |_|   /_/   \_\ |_| \_\   |_|    "
// "                                                                                                                       "
// "======================================================================================================================="

/// DAP server responded to our `initialize` request
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::OnInitializedEvent(DAPEvent & event)
// ----------------------------------------------------------------------------
{
    // got initialized event, place breakpoints and continue
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
    dap::InitializeResponse * response_data = event.GetDapResponse()->As<dap::InitializeResponse>();

    if (response_data)
    {
        if (response_data->success)
        {
            // Setup initial data watches
            CreateStartWatches();
        }
    }
}

/// Received a response to `GetFrames()` call
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::OnScopes(DAPEvent & event)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
    dap::ScopesResponse * resp = event.GetDapResponse()->As<dap::ScopesResponse>();

    m_RequestedGPRegs = m_RequestedFloats = false; //(ph 2024/08/10)

    if (resp)
    {
        m_stackdapvariables.clear();

        for (const dap::Scope & scope : resp->scopes)
        {
            if (
                //(scope.name.IsSameAs("Locals", false) /* &&  scope.presentationHint.IsSameAs("Locals", false) */)
                //||
                (scope.name.IsSameAs("Globals", false))
                ||
                (scope.name.IsSameAs("Registers", false) /*&&  scope.presentationHint.IsSameAs("Registers", false)*/)
            )
            {
                //-m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Do not request variables for %s."), scope.name), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
                if (scope.name.IsSameAs("Registers", /*matchCase*/false))
                {
                    // If the window is not shown, no need to ask for "CPU Registers"
                    // FYI: Do not use IsWindowIsReallyShown(). It reports false on first showing.
                    wxWindow* pWindow = Manager::Get()->GetDebuggerManager()->GetCPURegistersDialog()->GetWindow();
                    if (pWindow and pWindow->IsShownOnScreen())
                    {
                        // Log the fact that we're requesting the register values
                        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Requesting Register values."), dbg_DAP::LogPaneLogger::LineType::UserDisplay);

                        // Use the variablesReference from the "Registers" scope to request the actual register values
                        m_pDAPClient->GetChildrenVariables(scope.variablesReference, dap::EvaluateContext::VARIABLES, 0);
                    }
                }
            }
            else
            {
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Requesting Variable values."), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
                m_pDAPClient->GetChildrenVariables(scope.variablesReference, dap::EvaluateContext::VARIABLES, 0);
            }
        }
    }
}

// ----------------------------------------------------------------------------
bool DBG_DAP_Watches::AddWatchChildByRequestSequence(cb::shared_ptr<dbg_DAP::DAPWatch> pWatch, int requestSeq, const dap::Variable & var)
// ----------------------------------------------------------------------------
{
    if (pWatch->GetDAPChildVariableRequestSequence() == requestSeq)
    {
#ifdef DAP_DEBUG_ENABLE
        wxString attributes = wxEmptyString;

        for (const auto & attrib : var.presentationHint.attributes)
        {
            attributes += " " + attrib;
        }

        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                 __LINE__,
                                 wxString::Format(_("Child found for request #%d   Var: %s  (%d) = %s , Type: %s, Hint: kind: %s , attributes %s , visibility: %s "),
                                                  requestSeq,
                                                  var.name,
                                                  var.variablesReference,
                                                  var.value,
                                                  var.type,
                                                  var.presentationHint.kind,
                                                  attributes,
                                                  var.presentationHint.visibility
                                                 ),
                                 dbg_DAP::LogPaneLogger::LineType::UserDisplay);
#endif
        if (var.name.empty())
        {
            // DAP is returning a child item with an empty name which asserts when appending to watches window
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Attempt to add unnamed watch child; setting to 'anonymous'"), dbg_DAP::LogPaneLogger::LineType::Error);
            const_cast<dap::Variable &>(var).name = "anonymous";
        }

        cb::shared_ptr<dbg_DAP::DAPWatch> childWatch(new dbg_DAP::DAPWatch(m_pProject, m_pLogger, var.name, false));
        childWatch->SetValue(var.value);
        childWatch->SetDAPVariableReference(var.variablesReference);
        childWatch->SetType(var.type);

        if (var.variablesReference > 0)
        {
            childWatch->SetHasBeenExpanded(false);
            childWatch->SetRangeArray(0, var.variablesReference);
            cbWatch::AddChild(childWatch, cb::shared_ptr<cbWatch>(new dbg_DAP::DAPWatch(m_pProject, m_pLogger, "updating...", false)));
        }

        cbWatch::AddChild(pWatch, childWatch);
        return true;
    }
    else
    {
        if (pWatch->GetChildCount() > 0)
        {
            int childcount = pWatch->GetChildCount();

            for (int child = 0; child < childcount; ++child)
            {
                cb::shared_ptr<dbg_DAP::DAPWatch> cWatch = cb::static_pointer_cast<dbg_DAP::DAPWatch>(pWatch->GetChild(child));

                if (var.name.empty())
                {
                    // DAP is returning a child item with an empty name which asserts when appending to watches window
                    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Attempt to add unnamed watch child; setting to 'anonymous'"), dbg_DAP::LogPaneLogger::LineType::Error);
                    const_cast<dap::Variable &>(var).name = "anonymous";
                }

                if (AddWatchChildByRequestSequence(cWatch, requestSeq, var))
                {
                    return true;
                }
            }
        }

        return false;
    }
}
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::AppendNullChild(cbWatch::Pointer watch)
// ----------------------------------------------------------------------------
{
    // Add a place holder for children  (ph 25/04/19)
    // cbWatch::AddChild(watch, cbWatch::Pointer(new Watch(_T("updating..."), watch->ForTooltip(), watch->GetProject())));
    cb::shared_ptr<dbg_DAP::DAPWatch> childWatch(new dbg_DAP::DAPWatch(m_pProject, m_pLogger, "...", false));
    childWatch->SetValue(_("Updating..."));
    //?childWatch->SetDAPVariableReference(var.variablesReference);
    //?childWatch->SetType(var.type);
    childWatch->SetHasBeenExpanded(false);
    //?childWatch->SetRangeArray(0, var.variablesReference);
    cbWatch::AddChild(childWatch, watch);
}
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::OnVariablesCheck(DAPEvent &event)
// ----------------------------------------------------------------------------
{

    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
    dap::VariablesResponse * resp = event.GetDapResponse()->As<dap::VariablesResponse>();

    if (resp)
    {
        for (const dap::Variable& var : resp->variables)
        {
            m_stackdapvariables.push_back(var);
#ifdef DAP_DEBUG_ENABLE
            wxString button = (var.variablesReference > 0 ? "> " : "  ");
            wxString value = var.value.empty() ? "\"\"" : var.value;
            wxString attributes = wxEmptyString;

            for (const auto & attrib : var.presentationHint.attributes)
            {
                attributes += " " + attrib;
            }

            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                     __LINE__,
                                     wxString::Format(_("Var: %s  (%d) %s = %s , Type: %s, Hint: kind: %s , attributes %s , visibility: %s "),
                                                      button,
                                                      var.variablesReference,
                                                      var.name,
                                                      value,
                                                      var.type,
                                                      var.presentationHint.kind,
                                                      attributes,
                                                      var.presentationHint.visibility
                                                     ),
                                     dbg_DAP::LogPaneLogger::LineType::UserDisplay);
#endif
        }

        for (dbg_DAP::DAPWatchesContainer::iterator it = m_DAP_watches.begin(); it != m_DAP_watches.end(); ++it)
        {
            wxString symbol = (*it)->GetSymbol();

            for (const dap::Variable & var : resp->variables)
            {
                //wxString varname = var.name;
                if (symbol.IsSameAs(var.name))
                {
                    wxString value = var.value.empty() ? "\"\"" : var.value;
                    (*it)->SetValue(value);
                    (*it)->SetType(var.type);
                    (*it)->SetDAPVariableReference(var.variablesReference); // (ph 25/04/19)
                    (*it)->SetDAPChildVariableRequestSequence(resp->request_seq); // (ph 25/04/19)
                }
                //?else // (ph 25/04/19)
                if (var.variablesReference) //if any children indicated from DAP server? // (ph 25/04/19)
                {
                    AddWatchChildByRequestSequence(*it, resp->request_seq, var);
                }
            }
        }

        UpdateDAPWatches(int(cbDebuggerPlugin::DebugWindows::Watches));
    }

}
// ----------------------------------------------------------------------------
void DBG_DAP_Watches::OnVariables(DAPEvent &event)
// ----------------------------------------------------------------------------
{

    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
    dap::VariablesResponse *resp = event.GetDapResponse()->As<dap::VariablesResponse>();

    if (resp)
    {
        std::map<wxString, wxString> localVariables;
        std::map<wxString, wxString> cpuRegisterVariables;

        for (const dap::Variable &var : resp->variables)
        {
            wxString value = var.value.empty() ? "\"\"" : var.value;

            if (var.name.EndsWith("Registers") || var.presentationHint.kind == "register")
            {
                // It's a register
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__,
                                         wxString::Format("RequestedGPRegs:%s RequestedFloats:%s", m_RequestedGPRegs ? "True" : "False", m_RequestedFloats ? "True" : "False"));

                wxString button = (var.variablesReference > 0 ? "> " : "  ");

                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                         __LINE__,
                                         wxString::Format(_("Var: %s  (%d) %s = %s , Type: %s, Hint: kind: %s , attributes %s , visibility: %s "),
                                                          button,
                                                          var.variablesReference,
                                                          var.name,
                                                          value,
                                                          var.type,
                                                          var.presentationHint.kind,
                                                          wxEmptyString,
                                                          var.presentationHint.visibility),
                                         dbg_DAP::LogPaneLogger::LineType::UserDisplay);

////                if (not var.value.IsEmpty())
////                    registerVariables[var.name] = value;

                if (var.name == "General Purpose Registers")
                {
                    // FIXME (ph#): Don't need to do this if Register window not shown
                    // Request detailed register values for these categories
                    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Requesting values for %s with variablesReference %d."), var.name, var.variablesReference), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
                    m_pDAPClient->GetChildrenVariables(var.variablesReference, dap::EvaluateContext::VARIABLES, 0);
                    m_RequestedGPRegs = true;
                }
                else if (var.name == "Floating Point Registers")
                {
                    // FIXME (ph#): Don't need to do this if Register window not shown
                    // Request detailed register values for these categories
                    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Requesting values for %s with variablesReference %d."), var.name, var.variablesReference), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
                    m_pDAPClient->GetChildrenVariables(var.variablesReference, dap::EvaluateContext::VARIABLES, 0);
                    m_RequestedFloats = true;
                }
            }
            if (m_RequestedGPRegs or m_RequestedFloats)
            {
                // It's a register variable but ignore non-valued items
                if (not var.value.IsEmpty())
                    cpuRegisterVariables[var.name] = value;
            }
            else
            {
                // It's a local variable
                localVariables[var.name] = value;
            }

////            if (not (m_RequestedGPRegs and m_RequestedFloats))
////            {
////                // Push all non-register variables for later use
////                m_stackdapvariables.push_back(var);
////                m_stackdapVarReqSeq.push_back(resp->request_seq); // (ph 25/04/12)
////            }

          #ifdef DAP_DEBUG_ENABLE
            wxString button = (var.variablesReference > 0 ? "> " : "  ");
            wxString attributes = wxEmptyString;

            for (const auto &attrib : var.presentationHint.attributes)
            {
                attributes += " " + attrib;
            }

            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                     __LINE__,
                                     wxString::Format(_("Var: %s  (%d) %s = %s , Type: %s, Hint: kind: %s , attributes %s , visibility: %s "),
                                                      button,
                                                      var.variablesReference,
                                                      var.name,
                                                      value,
                                                      var.type,
                                                      var.presentationHint.kind,
                                                      attributes,
                                                      var.presentationHint.visibility),
                                     dbg_DAP::LogPaneLogger::LineType::UserDisplay);
          #endif // DAP_DEBUG_ENABLE
        }//end for resp->variables

        // Process local variables and register variables separately
        for (const auto &localVar : localVariables)
        {
            // Handle local variables
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("LocalVar: %s = %s", localVar.first, localVar.second), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
        }

        for (const auto& regVar : cpuRegisterVariables)
        {
            // Log register variables
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("RegVar: %s = %s", regVar.first, regVar.second), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
        }

        // Process watches
        if (localVariables.size())
            OnVariablesCheck(event);

        // Process CPU Registers //(ph 2024/08/10)
        if (cpuRegisterVariables.size())
            m_pInforRegisters->OnVarCPURegisters(cpuRegisterVariables);

        // Display watches. disassembly, memory
//?        UpdateDAPWatches(int(cbDebuggerPlugin::DebugWindows::Watches));
    } // endif resp
}
// ----------------------------------------------------------------------------
// implement class InfoRegisters : public Action
// ----------------------------------------------------------------------------
// only tested on mingw/pc/win env
DBG_DAP_InfoRegisters::DBG_DAP_InfoRegisters(cbDebuggerPlugin* plugin, dbg_DAP::LogPaneLogger* logger,  wxString disassemblyFlavor)
    : m_plugin(plugin),
      m_pLogger(logger),
      m_disassemblyFlavor(disassemblyFlavor),
      m_parsed_reg_names(false),
      m_parsed_reg_values(false)
{
}
// ----------------------------------------------------------------------------
DBG_DAP_InfoRegisters::~DBG_DAP_InfoRegisters()
// ----------------------------------------------------------------------------
{
}

// ----------------------------------------------------------------------------
void DBG_DAP_InfoRegisters::OnVarCPURegisters(std::map<wxString,wxString>& registerVariablesMap)
// ----------------------------------------------------------------------------
{
    if (registerVariablesMap.size())
    {

        if(not registerVariablesMap.size())
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, ("No register-values in the output"));
        else
        {
            m_parsed_reg_names = m_parsed_reg_values = true;

            // or32 register string parser
            if(m_disassemblyFlavor == _T("set disassembly-flavor or32"))
            {
                //-ParseOutputFromOR32gdbPort(output);
            }
            else    // use generic parser - this may work for other platforms or you may have to write your own
            {
                int isize = registerVariablesMap.size();
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__,
                                 wxString::Format("InfoRegisters size: %d",isize),dbg_DAP::LogPaneLogger::LineType::UserDisplay);

                regNames.Clear();
                regValues.Clear();

                for (const auto& regItem : registerVariablesMap)
                {
                    wxString regName = regItem.first;
                    wxString value = regItem.second;
                    regNames.Add(regName);
                    regValues.Add(value);
                }
            }//endElse
        }//endElse
    }//endIf registerVariablesMap.size

    if (m_parsed_reg_names && m_parsed_reg_values)
    {
        cbCPURegistersDlg* pDialog = Manager::Get()->GetDebuggerManager()->GetCPURegistersDialog();
        // Do not clear(). dialog has prevous General registers data from prior OnVariables() event.
        // Because the stupid DAP does not separate events for vars, then GPRs, then Floats values.
        for ( unsigned ii = 0;  (ii < regNames.GetCount()) and ( ii < regValues.GetCount()); ++ii )
        {
            unsigned long int addrL;
            wxString addr = regValues[ii];
            addr.ToULong(&addrL, 16);
            pDialog->SetRegisterValue(regNames[ii], addr, wxEmptyString);
            if (regNames[ii] == "eflags")
            {
                wxString hexStr = BinStrToHexStr(regValues[ii]);
                if (not hexStr.empty())
                    pDialog->SetRegisterValue(regNames[ii], hexStr, wxString());
            }
        }
    }

}//end OnVarRegisters

#include <bitset>
// ----------------------------------------------------------------------------
wxString DBG_DAP_InfoRegisters::BinStrToHexStr(const wxString &binaryStr)
// ----------------------------------------------------------------------------
{
    // Check if the binary string has a "0b" prefix
    if (!binaryStr.StartsWith("0b"))
    {
        //throw std::invalid_argument("Invalid binary string format. Expected prefix '0b'.");
        return wxString();
    }

    // Remove the "0b" prefix
    wxString binary = binaryStr.Mid(2);

    // Ensure the binary string length is a multiple of 4 by padding with leading zeros
    while (binary.length() % 4 != 0) {
        binary = "0" + binary;
    }

     wxString hexStr = "0x";  // Start with '0x' prefix

    // Convert each 4-bit segment to a hexadecimal digit
    for (size_t i = 0; i < binary.length(); i += 4) {
        wxString nibble = binary.Mid(i, 4);
        std::bitset<4> bits(std::string(nibble.mb_str()));
        hexStr << wxString::Format("%X", bits.to_ulong());
    }

    return hexStr;
}

// "==================================================================================================================="
// "          ____       _      ____      _____  __     __  _____   _   _   _____   ____      _____   _   _   ____     "
// "         |  _ \     / \    |  _ \    | ____| \ \   / / | ____| | \ | | |_   _| / ___|    | ____| | \ | | |  _ \    "
// "         | | | |   / _ \   | |_) |   |  _|    \ \ / /  |  _|   |  \| |   | |   \___ \    |  _|   |  \| | | | | |   "
// "         | |_| |  / ___ \  |  __/    | |___    \ V /   | |___  | |\  |   | |    ___) |   | |___  | |\  | | |_| |   "
// "         |____/  /_/   \_\ |_|       |_____|    \_/    |_____| |_| \_|   |_|   |____/    |_____| |_| \_| |____/    "
// "                                                                                                                   "
// "==================================================================================================================="
