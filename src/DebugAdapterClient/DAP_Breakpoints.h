/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
*/

#ifndef __DAP_BREAKPOINTS_H__
#define __DAP_BREAKPOINTS_H__

// System and library includes
//-#include <memory> unused directly
#include <wx/menu.h>
#include <wx/timer.h>
#include <wx/wxprec.h>

// CB includes
#include <cbplugin.h>
#include <tinyxml2.h>

// DAP debugger includes
#include "debugger_logger.h"
#include "definitions.h"

// DAP protocol includes
#include "Client.hpp"
//#include "Process.hpp"
#include "dlg_SettingsOptions.h"

class TextCtrlLogger;
class Compiler;

class DBG_DAP_Breakpoints
{
    public:
        /** Constructor. */
        DBG_DAP_Breakpoints(cbDebuggerPlugin* plugin, dbg_DAP::LogPaneLogger* logger, dap::Client* pDAPClient);
        /** Destructor. */
        ~DBG_DAP_Breakpoints();

        dbg_DAP::DebuggerConfiguration & GetActiveConfigEx();

        // breakpoints calls
        cb::shared_ptr<cbBreakpoint> AddBreakpoint(const wxString & filename, int line);
        cb::shared_ptr<cbBreakpoint> UpdateOrAddBreakpoint(const wxString & filename, const int line, const bool bEnable, const int id);
        //        cb::shared_ptr<cbBreakpoint> AddBreakpoint(cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp);
        cb::shared_ptr<cbBreakpoint> AddDataBreakpoint(const wxString & dataExpression);
        int GetBreakpointsCount() const;
        cb::shared_ptr<cbBreakpoint> GetBreakpoint(int index);
        cb::shared_ptr<const cbBreakpoint> GetBreakpoint(int index) const;
        cb::shared_ptr<cbBreakpoint> GetBreakpointByID(int id);
        void UpdateBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint);
        void DeleteBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint);
        void DeleteAllBreakpoints();
        void ShiftBreakpoint(int index, int lines_to_shift) {};
        bool ShiftAllFileBreakpoints(const wxString & filename, int startline, int lines);
        void EnableBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint, bool bEnable);

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

        bool CompilerFinished(bool compilerFailed, cbDebuggerPluginExt::StartType startType);
        void UpdateDebugDialogs(bool bClearAllData);
        void OnProjectOpened(CodeBlocksEvent & event);
        void CleanupWhenProjectClosed(cbProject * project);

        // breakpoints
        void CreateStartBreakpoints(bool force);
        void UpdateMapFileBreakPoints(const wxString & filename, cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp, bool bAddBreakpoint);
        void UpdateDAPSetBreakpointsByFileName(const wxString & filename);
        void RunToCursor(const wxString & filename, int line, const wxString & line_text);

        // misc
        void DAPDebuggerResetData(dbg_DAP::ResetDataType bClearAllData);
        bool SaveStateToFile(cbProject * prj);
        bool LoadStateFromFile(cbProject * prj);
        void OnProcessBreakpointData(const wxString & brkDescription);
        void SetDebuggee(const wxString & debuggee);
        void SetProject(cbProject * m_pProject);

        /// Dap events
        void OnInitializedEvent(DAPEvent & event);
        void OnBreakpointDataSet(DAPEvent & event);
        void OnInitializeResponse(DAPEvent & event);

        cbDebuggerPluginExt::StartType m_start_type = cbDebuggerPluginExt::StartTypeUnknown; //(ph 2024/06/07)

    private:
        cbDebuggerPlugin * m_plugin;
        cbProject * m_pProject;
        dbg_DAP::LogPaneLogger * m_pLogger;
        dap::Client * m_pDAPClient;
        wxString m_debuggee;

        // breakpoints
        dbg_DAP::DAPBreakpointsContainer m_breakpoints;
        std::map<wxString, std::vector<cb::shared_ptr<dbg_DAP::DAPBreakpoint>>> m_map_filebreakpoints;
        std::map<wxString, wxString> m_map_fileSystemDap;
        dbg_DAP::DAPBreakpointsContainer m_temporary_breakpoints;
        cb::shared_ptr<dbg_DAP::DAPBreakpoint> FindBreakpoint(const cbProject * project, const wxString & filename, const int line);

        // Available exception filter options for the 'setExceptionBreakpoints' request.
        std::vector<dap::ExceptionBreakpointsFilter> vExceptionBreakpointFilters;
};

#endif // __DAP_BREAKPOINTS_H__
