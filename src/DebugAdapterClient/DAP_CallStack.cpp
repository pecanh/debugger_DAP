/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
*/
#define DAP_DEBUG_ENABLE 1

// CB include files (not DAP)
#include "cbdebugger_interfaces.h"
#include "cbplugin.h"
#include "cbproject.h"
//#include "compilerfactory.h"

// DAP include files
#include "helpers.h"
#include "DAP_CallStack.h"
#include "dlg_SettingsOptions.h"
#include "debugger_logger.h"
#include "DAP_Debugger_State.h"

// constructor
// ----------------------------------------------------------------------------
DBG_DAP_CallStack::DBG_DAP_CallStack(cbDebuggerPlugin * plugin, dbg_DAP::LogPaneLogger * logger, dap::Client* pDAPClient) :
// ----------------------------------------------------------------------------
    m_plugin(plugin),
    m_pDAPClient(pDAPClient),
    m_pLogger(logger)
{
}

// destructor
// ----------------------------------------------------------------------------
DBG_DAP_CallStack::~DBG_DAP_CallStack()
// ----------------------------------------------------------------------------
{
}

// ----------------------------------------------------------------------------
void DBG_DAP_CallStack::OnAttachReal()
// ----------------------------------------------------------------------------
{
    Manager::Get()->GetLogManager()->DebugLog(wxString::Format("%s %d", __PRETTY_FUNCTION__, __LINE__));
}

// ----------------------------------------------------------------------------
void DBG_DAP_CallStack::OnReleaseReal(bool appShutDown)
// ----------------------------------------------------------------------------
{
    // Do not log anything as we are closing
    DAPDebuggerResetData(dbg_DAP::ResetDataType::ResetData_All);
}

dbg_DAP::DebuggerConfiguration & DBG_DAP_CallStack::GetActiveConfigEx()
{
    return static_cast<dbg_DAP::DebuggerConfiguration &>(m_plugin->GetActiveConfig());
}

// ----------------------------------------------------------------------------
void DBG_DAP_CallStack::UpdateDebugDialogs(bool bClearAllData)
// ----------------------------------------------------------------------------
{
    cbBacktraceDlg * pDialogBacktrace = Manager::Get()->GetDebuggerManager()->GetBacktraceDialog();

    if (pDialogBacktrace)
    {
        pDialogBacktrace->Reload();
    }

    cbThreadsDlg * pDialogThreads = Manager::Get()->GetDebuggerManager()->GetThreadsDialog();

    if (pDialogThreads)
    {
        pDialogThreads->Reload();
    }
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
void DBG_DAP_CallStack::OnProjectOpened(CodeBlocksEvent & event)
// ----------------------------------------------------------------------------
{
    DAPDebuggerResetData(dbg_DAP::ResetDataType::ResetData_All);
}

// ----------------------------------------------------------------------------
void DBG_DAP_CallStack::CleanupWhenProjectClosed(cbProject * project)
// ----------------------------------------------------------------------------
{
    DAPDebuggerResetData(dbg_DAP::ResetDataType::ResetData_All);
}

// "===================================================================================================="
// "     ____    _____      _       ____   _  __       __    _____   ____       _      __  __   _____   "
// "    / ___|  |_   _|    / \     / ___| | |/ /      / /   |  ___| |  _ \     / \    |  \/  | | ____|  "
// "    \___ \    | |     / _ \   | |     | ' /      / /    | |_    | |_) |   / _ \   | |\/| | |  _|    "
// "     ___) |   | |    / ___ \  | |___  | . \     / /     |  _|   |  _ <   / ___ \  | |  | | | |___   "
// "    |____/    |_|   /_/   \_\  \____| |_|\_\   /_/      |_|     |_| \_\ /_/   \_\ |_|  |_| |_____|  "
// "                                                                                                    "
// "===================================================================================================="

// ----------------------------------------------------------------------------
int DBG_DAP_CallStack::GetStackFrameCount() const
// ----------------------------------------------------------------------------
{
    return m_backtrace.size();
}

// ----------------------------------------------------------------------------
cb::shared_ptr<const cbStackFrame> DBG_DAP_CallStack::GetStackFrame(int index) const
// ----------------------------------------------------------------------------
{
    return m_backtrace[index];
}

// ----------------------------------------------------------------------------
void DBG_DAP_CallStack::SwitchToFrame(int number)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("SwitchToFrame: %d"), number), dbg_DAP::LogPaneLogger::LineType::Debug);

    if (Debugger_State::IsRunning() && Debugger_State::IsStopped() && (number < static_cast<int>(m_backtrace.size())))
    {
        cb::shared_ptr<const cbStackFrame> frameToSwitch = m_backtrace[number];
        wxString sFileName = frameToSwitch->GetFilename();
        long int lineNumber;;
        frameToSwitch->GetLine().ToLong(&lineNumber);
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("SyncEditor: %s %ld"), sFileName, lineNumber), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
        m_plugin->SyncEditor(sFileName, lineNumber, true);
        //dbg_DAP::DAPCurrentFrame m_current_frame
         // update m_current_frame
        m_current_frame.DAPSwitchToFrame(number); //(ph 2024/05/28)
    }
}

// ----------------------------------------------------------------------------
int DBG_DAP_CallStack::GetActiveStackFrame() const
// ----------------------------------------------------------------------------
{
    return m_current_frame.GetStackFrame();
}

// "==============================================================================================="
// "     _____   _                                 _                                               "
// "    |_   _| | |__    _ __    ___    __ _    __| |  ___                                         "
// "      | |   | '_ \  | '__|  / _ \  / _` |  / _` | / __|                                        "
// "      | |   | | | | | |    |  __/ | (_| | | (_| | \__ \                                        "
// "      |_|   |_| |_| |_|     \___|  \__,_|  \__,_| |___/                                        "
// "                                                                                               "
// "==============================================================================================="

// ----------------------------------------------------------------------------
int DBG_DAP_CallStack::GetThreadsCount() const
// ----------------------------------------------------------------------------
{
    return m_threads.size();
}

// ----------------------------------------------------------------------------
cb::shared_ptr<const cbThread> DBG_DAP_CallStack::GetThread(int index) const
// ----------------------------------------------------------------------------
{
    return m_threads[index];
}
// ----------------------------------------------------------------------------
void DBG_DAP_CallStack::ClearThreads() //(ph 2024/06/15)
// ----------------------------------------------------------------------------
{
    m_threads.clear();
    Manager::Get()->GetDebuggerManager()->GetThreadsDialog()->Reload();

}
// ----------------------------------------------------------------------------
void DBG_DAP_CallStack::OnThreadsResponse(DAPEvent & event) //(ph 2024/06/15)
// ----------------------------------------------------------------------------
{
    m_threads.clear();
    dap::ThreadsResponse* threads_data = event.GetDapResponse()->As<dap::ThreadsResponse>();
    int current_thread_id = m_currentThreadID;

    if (threads_data)
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received threads event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
        m_threads.clear();


        for (const auto & thread : threads_data->threads)
        {
            #ifdef DAP_DEBUG_ENABLE
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                     __LINE__,
                                     //-wxString::Format(_("Stack: ID 0x%X , Name: %s , File: %s  %d"),
                                     wxString::Format(_("Stack: ID 0x%X , Name: %s "),
                                                      thread.id,
                                                      thread.name
                                                     ),
                                     dbg_DAP::LogPaneLogger::LineType::UserDisplay);
            #endif

            m_threads.push_back(cbThread::Pointer(new cbThread(thread.id == current_thread_id, thread.id, thread.name)));
        }

    }

    Manager::Get()->GetDebuggerManager()->GetThreadsDialog()->Reload();
}

// ----------------------------------------------------------------------------
bool DBG_DAP_CallStack::SwitchToThread(int thread_number)
// ----------------------------------------------------------------------------
{
    //DAP: issue a stack refresh with a thread number specified
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Functionality in progress"), dbg_DAP::LogPaneLogger::LineType::Error);
    if (Debugger_State::IsRunning() and Debugger_State::IsStopped()) //(ph 2024/06/13)
    {
        m_current_frame.Reset();
        m_current_frame.SetThreadId(thread_number);
        m_pDAPClient->GetFrames(thread_number);
        return true;

    }
    else
    {
        return false;
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

void DBG_DAP_CallStack::DAPDebuggerResetData(dbg_DAP::ResetDataType bClearAllData)
{
    m_backtrace.clear();
    m_current_frame.Reset();
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

// Received a response to `GetFrames()
//  call (called from plugin.cpp)
// ----------------------------------------------------------------------------
void DBG_DAP_CallStack::OnStackTrace(DAPEvent & event)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
    dap::StackTraceResponse* stack_trace_data = event.GetDapResponse()->As<dap::StackTraceResponse>();
    //wxString jsonStr = event.GetDapResponse()->ToString();
    if (stack_trace_data)
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received stack trace event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
        int stackID = 0;
        m_backtrace.clear();
        m_first_valid = -1; //(ph 2024/05/29)
        int cnt = -1;
        for (const auto& stack : stack_trace_data->stackFrames)
        {
#ifdef DAP_DEBUG_ENABLE
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                     __LINE__,
                                     wxString::Format(_("Stack: ID 0x%X , Name: %s , File: %s  %d %s"),
                                                      stack.id,
                                                      stack.name,
                                                      stack.source.path.IsEmpty() ? stack.source.name : stack.source.path,
                                                      stack.line,
                                                      stack.instructionPointerReference
                                                     ),
                                     dbg_DAP::LogPaneLogger::LineType::UserDisplay);
#endif
            cnt += 1;
            dbg_DAP::cbStackFrameXtn s; //FYI: This is the cbStackFrame extension class
            s.SetNumber(stackID++);
            s.SetFile(stack.source.path, wxString::Format("%d", stack.line));
            s.SetSymbol(stack.name);
            //-s.SetAddress(stack.id);
            s.SetDAPStackId(stack.id);   //(ph 2024/08/29)
            s.SetAddress(dbg_DAP::HexStrToBinary(stack.instructionPointerReference)); //(ph 2024/08/28)
            s.MakeValid(true);
            //-m_backtrace.push_back(cb::shared_ptr<cbStackFrame>(new cbStackFrame(s)));
            m_backtrace.push_back(cb::shared_ptr<dbg_DAP::cbStackFrameXtn>(new dbg_DAP::cbStackFrameXtn(s)));
            if(s.IsValid() && m_first_valid == -1)
                        m_first_valid = cnt;
        }

        cbBacktraceDlg* pDialogBacktrace = Manager::Get()->GetDebuggerManager()->GetBacktraceDialog();

        if (pDialogBacktrace)
        {
            // set m_current_frame before calling DialogBacktrace->Reload();
            // This code glommed from debugger_MI //(ph 2024/05/29)
            if (not m_backtrace.empty())
            {
                int frame = m_current_frame.GetUserSelectedFrame();
                if (frame < 0 && cbDebuggerCommonConfig::GetFlag(cbDebuggerCommonConfig::AutoSwitchFrame))
                    frame = m_first_valid;
                if (frame < 0)
                    frame = 0;
                m_current_frame.SetFrame(frame);

                int number = m_backtrace.empty() ? 0 : m_backtrace[frame]->GetNumber();
                if (m_old_active_frame != number)
                   SwitchToFrame(number);
            }

            pDialogBacktrace->Reload(); // This updates the visable call stack window
        }
    }
}


// "==================================================================================================================="
// "          ____       _      ____      _____  __     __  _____   _   _   _____   ____      _____   _   _   ____     "
// "         |  _ \     / \    |  _ \    | ____| \ \   / / | ____| | \ | | |_   _| / ___|    | ____| | \ | | |  _ \    "
// "         | | | |   / _ \   | |_) |   |  _|    \ \ / /  |  _|   |  \| |   | |   \___ \    |  _|   |  \| | | | | |   "
// "         | |_| |  / ___ \  |  __/    | |___    \ V /   | |___  | |\  |   | |    ___) |   | |___  | |\  | | |_| |   "
// "         |____/  /_/   \_\ |_|       |_____|    \_/    |_____| |_| \_|   |_|   |____/    |_____| |_| \_| |____/    "
// "                                                                                                                   "
// "==================================================================================================================="
