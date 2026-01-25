/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
*/

#ifndef __DAP_WATCHES_H__
#define __DAP_WATCHES_H__

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

// DAP protocol includes
#include "Client.hpp"
//-#include "Process.hpp"
#include "dlg_SettingsOptions.h"

class TextCtrlLogger;
class Compiler;
class DBG_DAP_InfoRegisters;
class DBG_DAP_CallStack; // (ph 25/04/06)

// ----------------------------------------------------------------------------
class DBG_DAP_Watches
// ----------------------------------------------------------------------------
{
    public:
        /** Constructor. */
        DBG_DAP_Watches(cbDebuggerPlugin* pPlugin, dbg_DAP::LogPaneLogger* logger, dap::Client* pDAPClient);
        /** Destructor. */
        ~DBG_DAP_Watches();

        dbg_DAP::DebuggerConfiguration & GetActiveConfigEx();

        // watches
        void CreateStartWatches();
        void UpdateDAPWatches(int updateType);
        cb::shared_ptr<cbWatch> AddWatch(const wxString& symbol, bool update);
        cb::shared_ptr<cbWatch> AddWatch(dbg_DAP::DAPWatch* watch, cb_unused bool update);
        /// No calls or interface exists for AddMemoryRange in either cdb/gdb or debuggerMI //(ph 2024/11/02)
        cb::shared_ptr<cbWatch> AddMemoryRange(uint64_t address, uint64_t size, const wxString & symbol, bool update);
        void DeleteWatch(cb::shared_ptr<cbWatch> watch);
        bool HasWatch(cb::shared_ptr<cbWatch> watch);
        bool IsMemoryRangeWatch(const cb::shared_ptr<cbWatch> & watch);
        void ShowWatchProperties(cb::shared_ptr<cbWatch> watch);
        bool SetWatchValue(cb::shared_ptr<cbWatch> watch, const wxString & value);
        void ExpandWatch(cb::shared_ptr<cbWatch> watch);
        void CollapseWatch(cb::shared_ptr<cbWatch> watch);
        void UpdateWatch(cb::shared_ptr<cbWatch> watch, DBG_DAP_CallStack* pDAPCallStack);
        void EvaluateWatchWithCallback(cb::shared_ptr<dbg_DAP::DAPWatch> real_watch, DBG_DAP_CallStack* pDAPCallStack); // (ph 25/04/06)
        void EvaluateWatchCallback(bool success, const wxString& result, const wxString& type, int variablesReference, const wxString& text, cb::shared_ptr<dbg_DAP::DAPWatch> real_watch);                   // (ph 25/04/06)

        void DoWatches();

        DBG_DAP_InfoRegisters* GetInfoRegisters() { return m_pInforRegisters; } //(ph 2024/08/13)

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

        void UpdateDebugDialogs(bool bClearAllData);
        void OnProjectOpened(CodeBlocksEvent & event);
        void CleanupWhenProjectClosed(cbProject * project);

        bool SaveStateToFile(cbProject * prj);
        bool LoadStateFromFile(cbProject * prj);

        /// Dap events
        void OnScopes(DAPEvent & event);
        void OnVariables(DAPEvent & event);
        void OnVariablesCheck(DAPEvent & event);
        void OnInitializedEvent(DAPEvent & event);
        void SetProject(cbProject * m_pProject);

        bool m_RequestedGPRegs = false;
        bool m_RequestedFloats = false;



    private:
        cbDebuggerPlugin * m_plugin;
        cbProject * m_pProject;
        dbg_DAP::LogPaneLogger * m_pLogger;
        dap::Client * m_pDAPClient;
        DBG_DAP_InfoRegisters* m_pInforRegisters = nullptr; //(ph 2024/08/13)

        // misc
        void DAPDebuggerResetData(dbg_DAP::ResetDataType bClearAllData);
        void OnProcessBreakpointData(const wxString & brkDescription);
        bool AddWatchChildByRequestSequence(cb::shared_ptr<dbg_DAP::DAPWatch> Watch, int requestSeq, const dap::Variable & var);
        void AppendNullChild(cbWatch::Pointer watch);

        // Watches
        dbg_DAP::DAPWatchesContainer m_DAP_watches;
        dbg_DAP::DAPMapWatchesToType m_mapWatchesToType;
        cb::shared_ptr<dbg_DAP::DAPWatch> m_WatchLocalsandArgs;
        std::vector<dap::Variable> m_stackdapvariables;
        //-std::vector<int> m_stackdapVarReqSeq;

};
// ----------------------------------------------------------------------------
class DBG_DAP_InfoRegisters // Registers stowage //(ph 2024/08/10)
// ----------------------------------------------------------------------------
{
    DBG_DAP_InfoRegisters(cbDebuggerPlugin* pPlugin, dbg_DAP::LogPaneLogger* logger, DBG_DAP_InfoRegisters&);
    DBG_DAP_InfoRegisters& operator =(DBG_DAP_InfoRegisters &);

    public:
        // only tested on mingw/pc/win env
        DBG_DAP_InfoRegisters(cbDebuggerPlugin* pPlugin, dbg_DAP::LogPaneLogger* logger, wxString disassemblyFlavor = wxEmptyString);
        virtual ~DBG_DAP_InfoRegisters();
        virtual void OnVarCPURegisters(std::map<wxString, wxString>& registerVariablesMAP);
        wxString BinStrToHexStr(const wxString &binaryStr);
        size_t GetRegistersCount(){return regNames.size();} //(ph 2024/08/30)

    protected:
        //-virtual void OnStart();
    private:
        cbDebuggerPlugin * m_plugin;
        dbg_DAP::LogPaneLogger* m_pLogger;
        wxString m_disassemblyFlavor;
        bool m_parsed_reg_names;
        bool m_parsed_reg_values;
        wxArrayString regNames;
        wxArrayString regValues;

}; //endClass InforRegisters

#endif // __DAP_WATCHES_H__
