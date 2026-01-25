/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
*/

#ifndef __DAP_CALLSTACK_H__
#define __DAP_CALLSTACK_H__

// System and library includes
//-#include <memory>
#include <wx/menu.h>
#include <wx/timer.h>
#include <wx/wxprec.h>

// CB includes
#include <cbplugin.h>
#include <tinyxml2.h>

// DAP debugger includes
#include "debugger_logger.h"
#include "definitions.h"
#include "dlg_SettingsOptions.h"

// DAP protocol includes
#include "Client.hpp"
//-#include "Process.hpp"

class TextCtrlLogger;
class Compiler;

class DBG_DAP_CallStack : public cbPlugin
{
    public:
        /** Constructor. */
        DBG_DAP_CallStack(cbDebuggerPlugin * plugin, dbg_DAP::LogPaneLogger * logger, dap::Client* pDAPClient);
        /** Destructor. */
        ~DBG_DAP_CallStack();

        dbg_DAP::DebuggerConfiguration & GetActiveConfigEx();

        // stack frame calls;
        int GetStackFrameCount() const;
        cb::shared_ptr<const cbStackFrame> GetStackFrame(int index) const;
        void SwitchToFrame(int number);
        int GetActiveStackFrame() const;

        // threads
        int GetThreadsCount() const;
        cb::shared_ptr<const cbThread> GetThread(int index) const;
        void ClearThreads();

        bool SwitchToThread(int thread_number);
        /** Any descendent plugin should override this method and
          * perform any necessary initialization. This method is called by
          * Code::Blocks (PluginManager actually) when the plugin has been
          * loaded and should attach in Code::Blocks. When Code::Blocks
          * starts up, it finds and <em>loads</em> all plugins but <em>does
          * not</em> activate (attaches) them. It then activates all plugins
          * that the user has selected to be activated on start-up.\n
          * This means that a plugin might be loaded but <b>not</b> activated...\n
          * Think of this method as the actual constructor...
          */
        void OnAttachReal();

        /** Any descendent plugin should override this method and
          * perform any necessary de-initialization. This method is called by
          * Code::Blocks (PluginManager actually) when the plugin has been
          * loaded, attached and should de-attach from Code::Blocks.\n
          * Think of this method as the actual destructor...
          * @param appShutDown If true, the application is shutting down. In this
          *         case *don't* use Manager::Get()->Get...() functions or the
          *         behaviour is undefined...
          */
        void OnReleaseReal(bool appShutDown);

        // misc
        void OnProjectOpened(CodeBlocksEvent & event);
        void CleanupWhenProjectClosed(cbProject * project);
        void DAPDebuggerResetData(dbg_DAP::ResetDataType bClearAllData);
        bool CompilerFinished(bool compilerFailed, cbDebuggerPluginExt::StartType startType);
        void UpdateDebugDialogs(bool bClearAllData);
        //(ph 2024/06/16)
        void SetCurrentThreadID(int threadID) {m_currentThreadID = threadID;}
        int current_thread_id = m_currentThreadID;

        /// DAP events
        void OnStackTrace(DAPEvent & event);
        void OnThreadsResponse(DAPEvent & event); //(ph 2024/06/14)


    private:
        cbDebuggerPlugin * m_plugin;
        dap::Client* m_pDAPClient; //(ph 2024/06/17)
        dbg_DAP::LogPaneLogger * m_pLogger;
        dbg_DAP::DAPThreadsContainer m_threads;

        // Stack
        dbg_DAP::DAPCurrentFrame m_current_frame;
        dbg_DAP::DAPBacktraceContainer m_backtrace;
        int m_first_valid = -1;     //(ph 2024/05/29)
        int m_old_active_frame = 0;  //(ph 2024/05/29)
        int m_currentThreadID = 0;     //(ph 2024/06/16)

};

#endif // __DAP_CALLSTACK_H__
