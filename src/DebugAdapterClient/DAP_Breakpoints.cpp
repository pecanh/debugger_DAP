/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
*/
#define DAP_DEBUG_ENABLE 1

// C++ include files
#include <algorithm>

// CB include files (not DAP)
#include "cbdebugger_interfaces.h"
#include "cbplugin.h"
#include "cbproject.h"
#include "compilerfactory.h"
#include "editormanager.h"
#include "cbeditor.h"

// DAP include files
#include "DAP_Breakpoints.h"
#include "dlg_SettingsOptions.h"
#include "debugger_logger.h"
#include "DAP_Debugger_State.h"
#include "dlg_editbreakpoint.h" //(ph 2024/06/10)
#include "dlg_databreakpoint.h" //(ph 2024/06/10)

//XML file root tag for data
static const char * XML_CFG_ROOT_TAG = "Debugger_layout_file";

// constructor
DBG_DAP_Breakpoints::DBG_DAP_Breakpoints(cbDebuggerPlugin* plugin, dbg_DAP::LogPaneLogger* logger, dap::Client * pDAPClient) :
    m_plugin(plugin),
    m_pProject(nullptr),
    m_pLogger(logger),
    m_pDAPClient(pDAPClient),
    m_debuggee(wxEmptyString)
{
}

// destructor
// ----------------------------------------------------------------------------
DBG_DAP_Breakpoints::~DBG_DAP_Breakpoints()
// ----------------------------------------------------------------------------
{
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::OnAttachReal()
// ----------------------------------------------------------------------------
{
    Manager::Get()->GetLogManager()->DebugLog(wxString::Format("%s %d", __PRETTY_FUNCTION__, __LINE__));
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::OnReleaseReal(bool appShutDown)
// ----------------------------------------------------------------------------
{
    // Do not log anything as we are closing
    DAPDebuggerResetData(dbg_DAP::ResetDataType::ResetData_All);
}

// ----------------------------------------------------------------------------
dbg_DAP::DebuggerConfiguration & DBG_DAP_Breakpoints::GetActiveConfigEx()
// ----------------------------------------------------------------------------
{
    return static_cast<dbg_DAP::DebuggerConfiguration &>(m_plugin->GetActiveConfig());
}

// ----------------------------------------------------------------------------
bool DBG_DAP_Breakpoints::CompilerFinished(bool compilerFailed, cbDebuggerPluginExt::StartType startType)
// ----------------------------------------------------------------------------
{
    if (compilerFailed || startType == cbDebuggerPluginExt::StartType::StartTypeUnknown)
    {
        m_temporary_breakpoints.clear();
    }

    return true;
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::UpdateDebugDialogs(bool bClearAllData)
// ----------------------------------------------------------------------------
{
    cbBreakpointsDlg * pDialogBreakpoint = Manager::Get()->GetDebuggerManager()->GetBreakpointDialog();

    if (pDialogBreakpoint)
    {
        if (bClearAllData)
        {
            pDialogBreakpoint->RemoveAllBreakpoints();
        }
        else
        {
            pDialogBreakpoint->Reload();
        }
    }
}
// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::SetDebuggee(const wxString & debuggee)
// ----------------------------------------------------------------------------
{
    m_debuggee = debuggee;
}
// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::SetProject(cbProject * pProject)
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
void DBG_DAP_Breakpoints::OnProjectOpened(CodeBlocksEvent & event)
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
void DBG_DAP_Breakpoints::CleanupWhenProjectClosed(cbProject * project)
// ----------------------------------------------------------------------------
{
    if (GetActiveConfigEx().GetFlag(dbg_DAP::DebuggerConfiguration::PersistDebugElements))
    {
        SaveStateToFile(project);
    }

    m_breakpoints.clear();
    m_map_filebreakpoints.clear();
    m_map_fileSystemDap.clear();
    cbBreakpointsDlg * dlg = Manager::Get()->GetDebuggerManager()->GetBreakpointDialog();
    dlg->Reload();
}

// "====================================================================================================="
// "  ____    _____   ____    _   _    ____        ____    ___    _   _   _____   ____     ___    _      "
// " |  _ \  | ____| | __ )  | | | |  / ___|      / ___|  / _ \  | \ | | |_   _| |  _ \   / _ \  | |     "
// " | | | | |  _|   |  _ \  | | | | | |  _      | |     | | | | |  \| |   | |   | |_) | | | | | | |     "
// " | |_| | | |___  | |_) | | |_| | | |_| |     | |___  | |_| | | |\  |   | |   |  _ <  | |_| | | |___  "
// " |____/  |_____| |____/   \___/   \____|      \____|  \___/  |_| \_|   |_|   |_| \_\  \___/  |_____| "
// "                                                                                                     "
// "====================================================================================================="

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::RunToCursor(const wxString & filename, int line, const wxString & /*line_text*/)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("RunToCursor %s:%d", filename, line), dbg_DAP::LogPaneLogger::LineType::Command);

    if (Debugger_State::IsRunning())
    {
        if (Debugger_State::IsStopped()) //(ph 2024/06/02)
        {
            // Add a temporary breakpoint (like below) and continue();
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("RunToCursor %s:%d", filename, line), dbg_DAP::LogPaneLogger::LineType::Command);
            cb::shared_ptr<cbBreakpoint> cbPtr = AddBreakpoint(filename,line);
            cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp = cb::static_pointer_cast<dbg_DAP::DAPBreakpoint>(cbPtr);
            bp->SetIsTemporary(true);
            m_pDAPClient->Continue();
        }
    }
    else // The menu needs to be enabled for this to work.
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("RunToCursor %s:%d", filename, line), dbg_DAP::LogPaneLogger::LineType::Command);
        cb::shared_ptr<cbBreakpoint> cbPtr = AddBreakpoint(filename,line);
        cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp = cb::static_pointer_cast<dbg_DAP::DAPBreakpoint>(cbPtr);
        bp->SetIsTemporary(true);
    }
}

//  "================================================================================================"
//  "      ____                          _                      _           _                        "
//  "     | __ )   _ __    ___    __ _  | | __  _ __     ___   (_)  _ __   | |_   ___                "
//  "     |  _ \  | '__|  / _ \  / _` | | |/ / | '_ \   / _ \  | | | '_ \  | __| / __|               "
//  "     | |_) | | |    |  __/ | (_| | |   <  | |_) | | (_) | | | | | | | | |_  \__ \               "
//  "     |____/  |_|     \___|  \__,_| |_|\_\ | .__/   \___/  |_| |_| |_|  \__| |___/               "
//  "                                          |_|                                                   "
//  "                                                                                                "
//  "================================================================================================"

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::CreateStartBreakpoints(bool force)
// ----------------------------------------------------------------------------
{
    long lineNumBrk;

    for (dbg_DAP::DAPBreakpointsContainer::iterator itBP = m_breakpoints.begin(); itBP != m_breakpoints.end(); ++itBP)
    {
        if ((*itBP)->GetLineString().ToLong(&lineNumBrk))
        {
            wxString brkFileName = (*itBP)->GetFilename();
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("File: %s  Line: %ld from m_breakpoints", brkFileName, lineNumBrk), dbg_DAP::LogPaneLogger::LineType::Debug);
            UpdateMapFileBreakPoints(brkFileName, (*itBP), true);
        }
    }

    for (dbg_DAP::DAPBreakpointsContainer::const_iterator itTBP = m_temporary_breakpoints.begin(); itTBP != m_temporary_breakpoints.end(); ++itTBP)
    {
        if ((*itTBP)->GetLineString().ToLong(&lineNumBrk))
        {
            wxString brkFileName = (*itTBP)->GetFilename();
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("File: %s  Line: %ld from m_temporary_breakpoints", brkFileName, lineNumBrk), dbg_DAP::LogPaneLogger::LineType::Debug);
            UpdateMapFileBreakPoints(brkFileName, (*itTBP), true);
        }
    }

    for (std::map<wxString, dbg_DAP::DAPBreakpointsContainer>::const_iterator itMFP = m_map_filebreakpoints.begin(); itMFP != m_map_filebreakpoints.end(); ++itMFP)
    {
        UpdateDAPSetBreakpointsByFileName(itMFP->first);
    }

    m_temporary_breakpoints.clear();
}

// ----------------------------------------------------------------------------
int DBG_DAP_Breakpoints::GetBreakpointsCount() const
// ----------------------------------------------------------------------------
{
    return m_breakpoints.size();
}

// ----------------------------------------------------------------------------
cb::shared_ptr<cbBreakpoint> DBG_DAP_Breakpoints::GetBreakpoint(int index)
// ----------------------------------------------------------------------------
{
    return cb::static_pointer_cast<cbBreakpoint>(m_breakpoints[index]);
}

// ----------------------------------------------------------------------------
cb::shared_ptr<const cbBreakpoint> DBG_DAP_Breakpoints::GetBreakpoint(int index) const
// ----------------------------------------------------------------------------
{
    return cb::static_pointer_cast<const cbBreakpoint>(m_breakpoints[index]);
}


// ----------------------------------------------------------------------------
cb::shared_ptr<cbBreakpoint> DBG_DAP_Breakpoints::GetBreakpointByID(int id)
// ----------------------------------------------------------------------------
{
    for (dbg_DAP::DAPBreakpointsContainer::iterator it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it)
    {
        //int seeid = (*it)->GetID(); // **Debugging**
        if ((*it)->GetID() == id)
        {
            return *it;
        }
    }

    return cb::shared_ptr<cbBreakpoint>();
}

// ----------------------------------------------------------------------------
cb::shared_ptr<dbg_DAP::DAPBreakpoint> DBG_DAP_Breakpoints::FindBreakpoint(const cbProject * project, const wxString & filename, const int line)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("filename: %s ,  Line %d"), filename, line), dbg_DAP::LogPaneLogger::LineType::Debug);

    for (dbg_DAP::DAPBreakpointsContainer::iterator it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it)
    {
        if (
            ((*it)->GetProject() == project)
            &&
            ((*it)->GetFilename() == filename)
            &&
            ((*it)->GetLine() == line)
        )
        {
            return *it;
        }
    }

    return cb::shared_ptr<dbg_DAP::DAPBreakpoint>();
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::UpdateMapFileBreakPoints(const wxString & filename, cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp, bool bAddBreakpoint)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("filename: %s ,  BP: %s Line %d  , bAddBreakpoint: %s"), filename, bp->GetFilename(), bp->GetLine(), bAddBreakpoint ? "True" : "False"), dbg_DAP::LogPaneLogger::LineType::Debug);
    std::map<wxString, dbg_DAP::DAPBreakpointsContainer>::iterator mapit;
    mapit = m_map_filebreakpoints.find(filename);

    if (mapit == m_map_filebreakpoints.end())
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("m_map_filebreakpoints includes filename %s"), filename), dbg_DAP::LogPaneLogger::LineType::Debug);

        if (bAddBreakpoint == true)
        {
            std::vector<cb::shared_ptr<dbg_DAP::DAPBreakpoint>> vecFileBPreakpoints;
            vecFileBPreakpoints.push_back(bp);
            m_map_filebreakpoints.insert(std::pair<wxString, std::vector<cb::shared_ptr<dbg_DAP::DAPBreakpoint>>>(filename, vecFileBPreakpoints));
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("m_map_filebreakpoints insert for filename %s ,  BP %s Line %d bAddBreakpoint: %s"), filename, bp->GetFilename(), bp->GetLine(), bAddBreakpoint ? "True" : "False"), dbg_DAP::LogPaneLogger::LineType::Debug);
        }
        else
        {
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Trying to remove a break point line, but it is not in m_map_filebreakpoints!"), dbg_DAP::LogPaneLogger::LineType::Error);
        }
    }
    else
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("m_map_filebreakpoints does not include filename %s"), filename), dbg_DAP::LogPaneLogger::LineType::Debug);
        bool bFoundLine = false;
        int lineBP = bp->GetLine();
        dbg_DAP::DAPBreakpointsContainer::iterator itFBrk;

        for (itFBrk = (*mapit).second.begin(); itFBrk != (*mapit).second.end(); ++itFBrk)
        {
            if ((*itFBrk)->GetLine() == lineBP)
            {
                bFoundLine = true;
                break;
            }
        }

        if (bFoundLine)
        {
            if (bAddBreakpoint)
            {
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Trying to add a break point line that is allready in m_map_filebreakpoints!"), dbg_DAP::LogPaneLogger::LineType::Error);
            }
            else
            {
                if ((*mapit).second.size() == 1)
                {
                    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("m_map_filebreakpoints erase filename %s ,  BP %s Line %d bAddBreakpoint: %s"), filename, bp->GetFilename(), bp->GetLine(), bAddBreakpoint ? "True" : "False"), dbg_DAP::LogPaneLogger::LineType::Debug);

                    if (m_map_filebreakpoints.size() == 1)
                    {
                        m_map_filebreakpoints.clear();
                    }
                    else
                    {
                        m_map_filebreakpoints.erase(mapit);
                    }
                }
                else
                {
                    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("m_map_filebreakpoints erase for filename %s ,  BP %s Line %d bAddBreakpoint: %s"), filename, bp->GetFilename(), bp->GetLine(), bAddBreakpoint ? "True" : "False"), dbg_DAP::LogPaneLogger::LineType::Debug);
                    (*mapit).second.erase(itFBrk);
                }
            }
        }
        else
        {
            if (bAddBreakpoint == true)
            {
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("m_map_filebreakpoints add filename %s ,  BP %s Line %d bAddBreakpoint: %s"), filename, bp->GetFilename(), bp->GetLine(), bAddBreakpoint ? "True" : "False"), dbg_DAP::LogPaneLogger::LineType::Debug);
                (*mapit).second.push_back(bp);
            }
            else
            {
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Trying to remove a break point line, but it is not in m_map_filebreakpoints!"), dbg_DAP::LogPaneLogger::LineType::Error);
            }
        }
    }
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::UpdateDAPSetBreakpointsByFileName(const wxString & filename)
// ----------------------------------------------------------------------------
{
    //-if (Debugger_State::IsRunning())
    if (Debugger_State::IsRunning() and Debugger_State::IsStopped()) //(ph 2024/06/08)
    {
        std::vector<dap::SourceBreakpoint> vlines;
        wxString sLineInfo = wxEmptyString;

        if (m_map_filebreakpoints.size() > 0)
        {
            std::map<wxString, dbg_DAP::DAPBreakpointsContainer>::iterator mapit;
            mapit = m_map_filebreakpoints.find(filename);
            dbg_DAP::DAPBreakpointsContainer filebreakpoints = mapit->second;

            for (dbg_DAP::DAPBreakpointsContainer::iterator it = filebreakpoints.begin(); it != filebreakpoints.end(); ++it)
            {
                if ((*it)->GetIsEnabled())
                {
                    int line = static_cast<int>((*it)->GetLine());
                    sLineInfo.Append(wxString::Format("%d ", line));
                    vlines.push_back({ line, wxEmptyString });
                }
            }
        }

        wxString fn;
        dbg_DAP::DebuggerConfiguration::eDebugTestingOptions debugtest = GetActiveConfigEx().GetDebugTestingChoiceOption();

        switch (debugtest)
        {
            default:
            case dbg_DAP::DebuggerConfiguration::eDebugTestingOptions::UseNativePaths:
                fn = UnixFilename(filename);
                break;

            case dbg_DAP::DebuggerConfiguration::eDebugTestingOptions::UseLinuxPathsOnWindowsWithDriveLetter:
                fn = UnixFilename(filename, wxPATH_UNIX);
                break;

            case dbg_DAP::DebuggerConfiguration::eDebugTestingOptions::UseLinuxPathsOnWindowsWithNoCollen:
            {
                fn = UnixFilename(filename, wxPATH_UNIX);
                int  ColonLocation = fn.Find(':');

                if (ColonLocation == 1)
                {
                    fn.Remove(ColonLocation, 1);
                    fn.Prepend("/");
                }
            }
            break;

            case dbg_DAP::DebuggerConfiguration::eDebugTestingOptions::UseLinuxPathsOnWindowsWithNoDriveLetter:
            {
                fn = UnixFilename(filename, wxPATH_UNIX);
                int  ColonLocation = fn.Find(':');

                if (ColonLocation == 1)
                {
                    fn.Remove(0, ColonLocation + 1);
                }
            }
            break;

            case dbg_DAP::DebuggerConfiguration::eDebugTestingOptions::UseWindowsPathsWithDriveLetterRemoved:
            {
                fn = UnixFilename(filename);
                int  ColonLocation = fn.Find(':');

                if (ColonLocation == 1)
                {
                    fn.Remove(0, ColonLocation + 1);
                }
            }
            break;

            case dbg_DAP::DebuggerConfiguration::eDebugTestingOptions::UseRelativePathComparedToTheExecutable:
            {
                fn = UnixFilename(filename);
                wxFileName fnFileName(fn);

                if (fnFileName.Exists())
                {
                    wxFileName fnDebuggee(m_debuggee);
                    fnFileName.MakeRelativeTo(fnDebuggee.GetPath());
                    fn = fnFileName.GetFullPath();
                }
            }
            break;
        }

        std::map<wxString, wxString>::iterator mapit;
        mapit = m_map_fileSystemDap.find(filename);

        if (mapit == m_map_fileSystemDap.end())
        {
            m_map_fileSystemDap.insert(std::pair<wxString, wxString>(filename, fn));
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("m_map_fileSystemDap insert for filename %s DAP filename %s"), filename, fn), dbg_DAP::LogPaneLogger::LineType::Debug);
        }

        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("m_pDAPClient->SetBreakpointsFile(%s , [%s]", fn, sLineInfo), dbg_DAP::LogPaneLogger::LineType::Debug);
        m_pDAPClient->SetBreakpointsFile(fn, vlines);
    }
}

// ----------------------------------------------------------------------------
cb::shared_ptr<cbBreakpoint> DBG_DAP_Breakpoints::AddBreakpoint(const wxString & filename, int line)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("AddBreakpoint %s:%d enabled"), filename, line), dbg_DAP::LogPaneLogger::LineType::Debug);
    return UpdateOrAddBreakpoint(filename, line, true, -1);
}

// ----------------------------------------------------------------------------
cb::shared_ptr<cbBreakpoint> DBG_DAP_Breakpoints::UpdateOrAddBreakpoint(const wxString & filename, const int line, const bool bEnable, const int id)
// ----------------------------------------------------------------------------
{
    wxString filenameBRK(filename);

    for (std::map<wxString, wxString>::const_iterator itMFSP = m_map_fileSystemDap.begin(); itMFSP != m_map_fileSystemDap.end(); ++itMFSP)
    {
        if (itMFSP->second.IsSameAs(filename))
        {
            filenameBRK = itMFSP->first;
            break;
        }
    }

    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Filename: %s Line: %d %s, ID: %d", filenameBRK, line, bEnable ? "Enable" : "Disable", id), dbg_DAP::LogPaneLogger::LineType::Debug);

    if (id != -1)
    {
        // Check if BP allready by ID
        for (dbg_DAP::DAPBreakpointsContainer::iterator itBP = m_breakpoints.begin(); itBP != m_breakpoints.end(); ++itBP)
        {
            if ((*itBP)->GetID() == id)
            {
                if ((*itBP)->GetIsEnabled() != bEnable)
                {
                    (*itBP)->SetIsEnabled(bEnable);
                    UpdateDAPSetBreakpointsByFileName(filenameBRK);
                }

                return *itBP;
            }
        }
    }

    cbProject * project = Manager::Get()->GetProjectManager()->FindProjectForFile(filenameBRK, nullptr, false, false);
    cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp = FindBreakpoint(project, filenameBRK, line);

    if (bp)
    {
        if (id != -1)
        {
            bp->SetID(id);
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Update %s:%d set ID %d"), filenameBRK, line, id), dbg_DAP::LogPaneLogger::LineType::Debug);
        }

        if (bp->GetIsEnabled() != bEnable)
        {
            bp->SetIsEnabled(bEnable);
            UpdateDAPSetBreakpointsByFileName(filenameBRK);
        }

        return bp;
    }
    else
    {
        cb::shared_ptr<dbg_DAP::DAPBreakpoint> newDAPBreakpoint(new dbg_DAP::DAPBreakpoint(project, m_pLogger, filenameBRK, line, id));
        UpdateMapFileBreakPoints(filenameBRK, newDAPBreakpoint, true);
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("m_breakpoints push_back %s:%d"), filenameBRK, line), dbg_DAP::LogPaneLogger::LineType::Debug);
        m_breakpoints.push_back(newDAPBreakpoint);
        UpdateDAPSetBreakpointsByFileName(filenameBRK);
        return newDAPBreakpoint;
    }
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::DeleteBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint)
// ----------------------------------------------------------------------------
{
    cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp = cb::static_pointer_cast<dbg_DAP::DAPBreakpoint>(breakpoint);
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("%s:%d"), bp->GetLocation(), bp->GetLine()), dbg_DAP::LogPaneLogger::LineType::Debug);
    dbg_DAP::DAPBreakpointsContainer::iterator it = std::find(m_breakpoints.begin(), m_breakpoints.end(), bp);

    if (it != m_breakpoints.end())
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Found %s:%d"), bp->GetLocation(), bp->GetLine()), dbg_DAP::LogPaneLogger::LineType::Debug);
        cb::shared_ptr<dbg_DAP::DAPBreakpoint> pBrkPt = *it;
        dbg_DAP::DAPBreakpoint::BreakpointType bpType = pBrkPt->GetType();

        switch (bpType)
        {
            case dbg_DAP::DAPBreakpoint::bptCode:
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Found dbg_DAP::DAPBreakpoint::bptCode breakpoint type")), dbg_DAP::LogPaneLogger::LineType::Debug);
                break;

            case dbg_DAP::DAPBreakpoint::bptData:
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Found dbg_DAP::DAPBreakpoint::bptData breakpoint type")), dbg_DAP::LogPaneLogger::LineType::Debug);
                return;

            case dbg_DAP::DAPBreakpoint::bptFunction:
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Found dbg_DAP::DAPBreakpoint::bptFunction breakpoint type")), dbg_DAP::LogPaneLogger::LineType::Debug);
                // #ifdef __MINGW32__
                //                 if (IsDebuggerPresent())
                //                 {
                //                     DebugBreak();
                //                 }
                //
                // #endif // __MINGW32__
                return;

            default:
                m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Unknown breakpoint type: %d",  bpType), dbg_DAP::LogPaneLogger::LineType::Error);
                // #ifdef __MINGW32__
                //
                //                 if (IsDebuggerPresent())
                //                 {
                //                     DebugBreak();
                //                 }
                //
                // #endif // __MINGW32__
                return;
        }

        wxString brkFileName = bp->GetLocation();
        UpdateMapFileBreakPoints(brkFileName, (*it), false);

        if (m_breakpoints.size() == 1)
        {
            m_breakpoints.clear(); // Delete after you have finished using *it
        }
        else
        {
            m_breakpoints.erase(it); // Delete after you have finished using *it
        }

        UpdateDAPSetBreakpointsByFileName(brkFileName);
    }

    cbBreakpointsDlg * dlg = Manager::Get()->GetDebuggerManager()->GetBreakpointDialog();
    dlg->Reload();
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::DeleteAllBreakpoints()
// ----------------------------------------------------------------------------
{
    if (Debugger_State::IsRunning())
    {
        std::vector<dap::SourceBreakpoint> vlines;
        vlines.clear();

        for (std::map<wxString, std::vector<cb::shared_ptr<dbg_DAP::DAPBreakpoint>>>::iterator it = m_map_filebreakpoints.begin(); it != m_map_filebreakpoints.end(); ++it)
        {
            m_pDAPClient->SetBreakpointsFile(it->first, vlines);
        }
    }

    m_map_filebreakpoints.clear();
    m_map_fileSystemDap.clear();
    m_breakpoints.clear();
    cbBreakpointsDlg * dlg = Manager::Get()->GetDebuggerManager()->GetBreakpointDialog();
    dlg->Reload();
}

// ----------------------------------------------------------------------------
cb::shared_ptr<cbBreakpoint> DBG_DAP_Breakpoints::AddDataBreakpoint(const wxString & dataExpression)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("dataExpression : %s", dataExpression), dbg_DAP::LogPaneLogger::LineType::Warning);

    dbg_DAP::DataBreakpointDlg dlg(Manager::Get()->GetAppWindow(), dataExpression, true, 1);
    PlaceWindow(&dlg);
    if (dlg.ShowModal() == wxID_OK)
    {
        bool enabled = dlg.IsBreakpointEnabled();
        const wxString& newDataExpression = dlg.GetDataExpression();
        int sel = dlg.GetSelection();

        cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp(new dbg_DAP::DAPBreakpoint(m_pProject, m_pLogger));
        bp->SetType(dbg_DAP::DAPBreakpoint::BreakpointType::bptData);
        bp->SetIsEnabled(enabled);
        bp->SetBreakAddress(newDataExpression);
        bp->SetIsBreakOnRead(sel != 1);
        bp->SetIsBreakOnWrite(sel != 0);

        //AddBreakpoint(bp);
        AddBreakpoint(bp->GetFilename(), bp->GetLine()); //(ph 2024/06/13)
        return bp;
    }
    else
    {
        return cb::shared_ptr<cbBreakpoint>();
    }
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::UpdateBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Functionality in progress!"), dbg_DAP::LogPaneLogger::LineType::Error);
    dbg_DAP::DAPBreakpointsContainer::iterator it = std::find(m_breakpoints.begin(), m_breakpoints.end(), breakpoint);

    if (it == m_breakpoints.end())
    {
        return;
    }

    cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp = cb::static_pointer_cast<dbg_DAP::DAPBreakpoint>(breakpoint);
    bool reset = false;
    switch (bp->GetType())
    {
        case dbg_DAP::DAPBreakpoint::bptCode:
        {
            dbg_DAP::EditBreakpointDlg dlg(*bp, Manager::Get()->GetAppWindow());
            PlaceWindow(&dlg);
            if (dlg.ShowModal() == wxID_OK)
            {
                *bp = dlg.GetBreakpoint();
                reset = true;
            }
            break;
        }
        case dbg_DAP::DAPBreakpoint::bptData:
        {
            int old_sel = 0;
            if (bp->GetIsBreakOnRead() && bp->GetIsBreakOnWrite())
            {
                old_sel = 2;
            }
            else if (!bp->GetIsBreakOnRead() && bp->GetIsBreakOnWrite())
            {
                old_sel = 1;
            }

            dbg_DAP::DataBreakpointDlg dlg(Manager::Get()->GetAppWindow(), bp->GetBreakAddress(), bp->GetIsEnabled(), old_sel);
            PlaceWindow(&dlg);
            if (dlg.ShowModal() == wxID_OK)
            {
                bp->SetIsEnabled(dlg.IsEnabled());
                bp->SetIsBreakOnRead(dlg.GetSelection() != 1);
                bp->SetIsBreakOnWrite(dlg.GetSelection() != 0);
                bp->SetBreakAddress(dlg.GetDataExpression());
                reset = true;
            }
            break;
        }
        case dbg_DAP::DAPBreakpoint::bptFunction:
            return;

        default:
            return;
    }

    if (reset)
    {
        //bool debuggerIsRunning = !IsStopped(); //(ph 2024/06/10)
        bool debuggerIsStopped= Debugger_State::IsStopped();



        //-if (debuggerIsRunning) //(ph 2024/06/10)
        if (not debuggerIsStopped)
        {
            //m_executor.Interupt(true); //(ph 2024/06/10)
            m_pDAPClient->Pause();
        }

        DeleteBreakpoint(bp);
        //-AddBreakpoint(bp); //(ph 2024/06/10)
        AddBreakpoint(bp->GetFilename(), bp->GetLine());

        //-if (debuggerIsRunning) //(ph 2024/06/10)
        if (debuggerIsStopped)
        {
            m_pDAPClient->Continue();
        }
    }
}

// ----------------------------------------------------------------------------
bool DBG_DAP_Breakpoints::ShiftAllFileBreakpoints(const wxString & editorFilename, int startline, int lines)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Filename: %s startline: %d lines move: %d", editorFilename, startline, lines), dbg_DAP::LogPaneLogger::LineType::Debug);
    wxString filenameBRK(editorFilename);

    for (std::map<wxString, wxString>::const_iterator itMFSP = m_map_fileSystemDap.begin(); itMFSP != m_map_fileSystemDap.end(); ++itMFSP)
    {
        if (itMFSP->second.IsSameAs(editorFilename))
        {
            filenameBRK = itMFSP->first;
            break;
        }
    }

    cbProject * project = Manager::Get()->GetProjectManager()->FindProjectForFile(filenameBRK, nullptr, false, false);
    bool bBrksUpdated = false;
    std::vector<cb::shared_ptr<dbg_DAP::DAPBreakpoint>> brksToRemove;

    for (dbg_DAP::DAPBreakpointsContainer::iterator itBP = m_breakpoints.begin(); itBP != m_breakpoints.end(); ++itBP)
    {
        int brkline = (*itBP)->GetLine();

        if (
            ((*itBP)->GetProject() == project)
            &&
            ((*itBP)->GetFilename() == filenameBRK)
            &&
            (brkline >= startline)
        )
        {
            if (lines < 0)
            {
                int endline = startline - lines - 1;

                if (brkline > endline)
                {
                    (*itBP)->SetShiftLines(lines);
                    bBrksUpdated = true;;
                }
                else
                {
                    if ((brkline >= startline) && (brkline <= endline))
                    {
                        brksToRemove.push_back(*itBP);
                        bBrksUpdated = true;;
                    }
                }
            }
            else
            {
                (*itBP)->SetShiftLines(lines);
                bBrksUpdated = true;;
            }
        }
    }

    for (std::vector<cb::shared_ptr<dbg_DAP::DAPBreakpoint>>::iterator it = brksToRemove.begin(); it != brksToRemove.end(); ++it)
    {
        DeleteBreakpoint(*it);
    }

    if (Debugger_State::IsRunning() && bBrksUpdated)
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format("Cannot move debugger Breakpoints as the debugger is running!!", editorFilename, startline, lines), dbg_DAP::LogPaneLogger::LineType::Error);
        //        UpdateDAPSetBreakpointsByFileName(filenameBRK);
    }

    return true;
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::EnableBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint, bool bEnable)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("%s:%d %s"), breakpoint->GetLocation(), breakpoint->GetLine(), bEnable ? "Enable" : "Disable"), dbg_DAP::LogPaneLogger::LineType::Debug);
    dbg_DAP::DAPBreakpointsContainer::iterator it = std::find(m_breakpoints.begin(), m_breakpoints.end(), breakpoint);

    if (it != m_breakpoints.end())
    {
        if ((*it)->IsEnabled() == bEnable)
        {
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                     __LINE__,
                                     wxString::Format(_("breakpoint found but no change needed: %s:%d"), breakpoint->GetLocation(), breakpoint->GetLine()),
                                     dbg_DAP::LogPaneLogger::LineType::Debug);
            return;
        }

        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                 __LINE__,
                                 wxString::Format(_("breakpoint found and change needed: %s:%d"), breakpoint->GetLocation(), breakpoint->GetLine()),
                                 dbg_DAP::LogPaneLogger::LineType::Debug);

        if (Debugger_State::IsRunning())
        {
            UpdateOrAddBreakpoint(breakpoint->GetLocation(), breakpoint->GetLine(), bEnable, (*it)->GetID());
        }

        (*it)->SetEnabled(bEnable);
    }
    else
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                 __LINE__,
                                 wxString::Format(_("Breakpoint NOT FOUND: %s:%d"), breakpoint->GetLocation(), breakpoint->GetLine()),
                                 dbg_DAP::LogPaneLogger::LineType::Error);
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

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::DAPDebuggerResetData(dbg_DAP::ResetDataType bClearAllData)
// ----------------------------------------------------------------------------
{
    if (bClearAllData == dbg_DAP::ResetDataType::ResetData_All)
    {
        m_breakpoints.clear();
    }

    if ((bClearAllData == dbg_DAP::ResetDataType::ResetData_Startup) || (bClearAllData == dbg_DAP::ResetDataType::ResetData_All))
    {
        m_map_filebreakpoints.clear();
        m_map_fileSystemDap.clear();
    }
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
bool DBG_DAP_Breakpoints::SaveStateToFile(cbProject * pProject)
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
    // ******************** Save breakpoints ********************
    tinyxml2::XMLElement * pElementBreakpointList = doc.NewElement("BreakpointsList");
    pElementBreakpointList->SetAttribute("count", static_cast<int64_t>(m_breakpoints.size()));
    tinyxml2::XMLNode * pBreakpointMasterNode = rootnode->InsertEndChild(pElementBreakpointList);

    for (dbg_DAP::DAPBreakpointsContainer::iterator it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it)
    {
        dbg_DAP::DAPBreakpoint & bp = **it;

        if (bp.GetProject() == pProject)
        {
            bp.SaveBreakpointToXML(pBreakpointMasterNode);
        }
    }

    // ********************  Save XML to disk ********************
    return doc.SaveFile(cbU2C(fname.GetFullPath()), false);
}

// ----------------------------------------------------------------------------
bool DBG_DAP_Breakpoints::LoadStateFromFile(cbProject * pProject)
// ----------------------------------------------------------------------------
{
    wxString projectFilename = pProject->GetFilename();

    if (projectFilename.IsEmpty())
    {
        return false;
    }

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

    // ******************** Load breakpoints ********************
    tinyxml2::XMLElement * pBreakpointList = root->FirstChildElement("BreakpointsList");

    if (pBreakpointList)
    {
        for (tinyxml2::XMLElement * pBreakpointElement = pBreakpointList->FirstChildElement("Breakpoint");
                pBreakpointElement;
                pBreakpointElement = pBreakpointElement->NextSiblingElement())
        {
            dbg_DAP::DAPBreakpoint * bpNew = new dbg_DAP::DAPBreakpoint(pProject, m_pLogger);
            //            bpNew->LoadBreakpointFromXML(pBreakpointElement, this);

            // Find new breakpoint in the m_breakpoints
            for (dbg_DAP::DAPBreakpointsContainer::iterator it = m_breakpoints.begin(); it != m_breakpoints.end(); ++it)
            {
                dbg_DAP::DAPBreakpoint & bpSearch = **it;

                if (
                    (bpSearch.GetProject() == bpNew->GetProject())   &&
                    (bpSearch.GetFilename() == bpNew->GetFilename()) &&
                    (bpSearch.GetLine() == bpNew->GetLine())
                )
                {
                    // Found breakpoint, so update it!!!
                    bpSearch.SetEnabled(bpNew->GetIsEnabled());
                    bpSearch.SetIsUseIgnoreCount(bpNew->GetIsUseIgnoreCount());
                    bpSearch.SetIgnoreCount(bpNew->GetIgnoreCount());
                    bpSearch.SetIsUseCondition(bpNew->GetIsUseCondition());
                    bpSearch.SetCondition(bpNew->GetCondition());
                }
            }
        }

        cbBreakpointsDlg * dlg = Manager::Get()->GetDebuggerManager()->GetBreakpointDialog();
        dlg->Reload();
    }

    // ******************** Finished Load ********************
    return true;
}

// "======================================================================================================================="
// "         ____       _      ____        ____    _   _   ____    ____     ___    ____    _____                           "
// "        |  _ \     / \    |  _ \      / ___|  | | | | |  _ \  |  _ \   / _ \  |  _ \  |_   _|                          "
// "        | | | |   / _ \   | |_) |     \___ \  | | | | | |_) | | |_) | | | | | | |_) |   | |                            "
// "        | |_| |  / ___ \  |  __/       ___) | | |_| | |  __/  |  __/  | |_| | |  _ <    | |                            "
// "        |____/  /_/   \_\ |_|         |____/   \___/  |_|     |_|      \___/  |_| \_\   |_|                            "
// "                                                                                                                       "
// "     _____   _   _   _   _    ____   _____   ___    ___    _   _   ____      ____    _____      _      ____    _____   "
// "    |  ___| | | | | | \ | |  / ___| |_   _| |_ _|  / _ \  | \ | | / ___|    / ___|  |_   _|    / \    |  _ \  |_   _|  "
// "    | |_    | | | | |  \| | | |       | |    | |  | | | | |  \| | \___ \    \___ \    | |     / _ \   | |_) |   | |    "
// "    |  _|   | |_| | | |\  | | |___    | |    | |  | |_| | | |\  |  ___) |    ___) |   | |    / ___ \  |  _ <    | |    "
// "    |_|      \___/  |_| \_|  \____|   |_|   |___|  \___/  |_| \_| |____/    |____/    |_|   /_/   \_\ |_| \_\   |_|    "
// "                                                                                                                       "
// "======================================================================================================================="

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::OnProcessBreakpointData(const wxString& brkDescription)
// ----------------------------------------------------------------------------
{
    if (not brkDescription.IsEmpty())
    {
        wxString::size_type brkLookupIndexStart;
        wxString brkLookup;
        wxString::size_type brkLookupIndexEnd;
        wxString brkID;
        wxString hitBreakText = "hitBreakpointIds\": [";

        if (brkDescription.Length() and brkDescription.Contains(hitBreakText))
        {
            int skipLength = hitBreakText.Length();
            brkLookupIndexStart = brkDescription.find(hitBreakText);
            //-if (brkLookupIndexStart != wxNOT_FOUND) // (ph 25/04/03)
            if (brkLookupIndexStart != wxString::npos)
                brkLookup = brkDescription.substr(brkLookupIndexStart + skipLength);
            if (not brkLookup.IsEmpty())
            {
                brkLookupIndexEnd = brkLookup.find(']');
                brkID = brkLookup.substr(0, brkLookupIndexEnd);
            }
        }
        else
        {
            brkLookupIndexStart = brkDescription.find(' ');
            brkDescription.substr(brkLookupIndexStart);
            if (not brkLookup.IsEmpty())
            {
                wxString::size_type brkLookupIndexEnd = brkLookup.find('.');
                wxString brkID = brkLookup.substr(0, brkLookupIndexEnd);
            }
        }

        if (not brkID.IsEmpty())
        {
            long id;

            if (brkID.ToLong(&id, 10))
            {
                cb::shared_ptr<cbBreakpoint> breakpoint = DBG_DAP_Breakpoints::GetBreakpointByID(id);

                if (breakpoint)
                {
                    cb::shared_ptr<dbg_DAP::DAPBreakpoint> bp = cb::static_pointer_cast<dbg_DAP::DAPBreakpoint>(breakpoint);

                    if (bp && (not bp->GetFilename().IsEmpty()))
                    {
                        //GetDAPCurrentFrame().SetPosition(bp.GetFilename(), bp.GetLine());
                        m_plugin->SyncEditor(bp->GetFilename(), bp->GetLine() + 1, true);
                        if (bp->GetIsTemporary())  //remove the temporary breakpoint //(ph 2024/06/03)
                            DeleteBreakpoint(bp);
                    }
                }
            }//endif (brkID.ToLong(&id, 10))
        }//endif (not brkID.IsEmpty())
    }//endif (not brkDescription.IsEmpty())

    m_pDAPClient->GetFrames();
    m_pDAPClient->GetThreads(); //(ph 2024/06/16)
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
void DBG_DAP_Breakpoints::OnInitializedEvent(DAPEvent & event)
// ----------------------------------------------------------------------------
{
    // got initialized event, place breakpoints and continue
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
    // Setup initial breakpoints
    CreateStartBreakpoints(true);

    // Set breakpoint on "main"
    //-if (GetActiveConfigEx().GetFlag(dbg_DAP::DebuggerConfiguration::StopOnMain))
    if (m_start_type == cbDebuggerPluginExt::StartTypeStepInto ) //(ph 2024/06/07)
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Placing breakpoint at main..."), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
        m_pDAPClient->SetFunctionBreakpoints({ { "main" } });
        // clear the start_type in case of any debugger restarts
        m_start_type = cbDebuggerPluginExt::StartTypeUnknown; //(ph 2024/06/07)
    }

    // Check for exception handling
    bool bExceptionCatch =  GetActiveConfigEx().GetFlag(dbg_DAP::DebuggerConfiguration::ExceptionCatch);
    bool bExceptionThrow =  GetActiveConfigEx().GetFlag(dbg_DAP::DebuggerConfiguration::ExceptionThrow);

    if (bExceptionCatch  || bExceptionThrow)
    {
        std::vector<wxString> vExceptionFilters;

        // Available exception filter options for the 'setExceptionBreakpoints' request.
        for (std::vector<dap::ExceptionBreakpointsFilter>::iterator it = vExceptionBreakpointFilters.begin(); it != vExceptionBreakpointFilters.end(); ++it)
        {
            if (
                ((*it).filter.IsSameAs("cpp_catch", false) && bExceptionCatch) ||
                ((*it).filter.IsSameAs("cpp_throw", false) && bExceptionThrow)
            )
            {
                vExceptionFilters.push_back((*it).filter);
            }
        }

        if (vExceptionFilters.size() > 0)
        {
            m_pDAPClient->SetExceptionBreakpoints(vExceptionFilters);
        }
    }
}


/// DAP server sent `initialize` reponse to our `initialize` message
// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::OnInitializeResponse(DAPEvent & event)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received event"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
    dap::InitializeResponse * response_data = event.GetDapResponse()->As<dap::InitializeResponse>();

    if (response_data)
    {
        if (response_data->success)
        {
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Got OnInitialize Response"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);

            if (!response_data->capabilities.exceptionBreakpointFilters.IsNull())
            {
                // Available exception filter options for the 'setExceptionBreakpoints' request.
                vExceptionBreakpointFilters = response_data->capabilities.exceptionBreakpointFilters.As<std::vector<dap::ExceptionBreakpointsFilter>>();

                for (std::vector<dap::ExceptionBreakpointsFilter>::iterator it = vExceptionBreakpointFilters.begin(); it != vExceptionBreakpointFilters.end(); ++it)
                {
                    if (!(*it).default_value.IsNull())
                    {
                        m_pLogger->LogDAPMsgType("", -1, wxString::Format(_("exceptionBreakpointFilters - filter:\"%s\"  label:\"%s\" value:%s"), (*it).filter, (*it).label, (*it).default_value.As<bool>() ? "True" : "False"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
                    }
                    else
                    {
                        m_pLogger->LogDAPMsgType("", -1, wxString::Format(_("exceptionBreakpointFilters - filter:\"%s\"  label:\"%s\" value:missing"), (*it).filter, (*it).label), dbg_DAP::LogPaneLogger::LineType::Error);
                    }
                }
            }
        }
        else
        {
            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Got BAD OnInitialize Response"), dbg_DAP::LogPaneLogger::LineType::Error);
        }
    }
}

// ----------------------------------------------------------------------------
void DBG_DAP_Breakpoints::OnBreakpointDataSet(DAPEvent & event)
// ----------------------------------------------------------------------------
{
    m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, _("Received response"), dbg_DAP::LogPaneLogger::LineType::UserDisplay);
    dap::SetBreakpointsResponse * resp = event.GetDapResponse()->As<dap::SetBreakpointsResponse>();

    if (resp)
    {
        m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__, __LINE__, wxString::Format(_("Got reply for setBreakpoint command for file: "), resp->originSource), dbg_DAP::LogPaneLogger::LineType::UserDisplay);

        for (const auto & bp : resp->breakpoints)
        {
            dbg_DAP::LogPaneLogger::LineType logType = dbg_DAP::LogPaneLogger::LineType::UserDisplay;

            if ((bp.line == -1) || !bp.verified) //  || bp.source.path.IsEmpty())
            {
                logType = dbg_DAP::LogPaneLogger::LineType::Error;
            }

            m_pLogger->LogDAPMsgType(__PRETTY_FUNCTION__,
                                     __LINE__,
                                     wxString::Format(_("ID %d , Verified: %s , File: %s , Line: %d, Message: %s"), bp.id, bp.verified ? "True" : "False", bp.source.path, bp.line, bp.message),
                                     logType
                                    );

            if ((bp.line != -1) && bp.verified && !bp.source.path.IsEmpty())
            {
                // deprecated by DAP. No longer returns source.path
                UpdateOrAddBreakpoint(bp.source.path, bp.line, true, bp.id);
            }
            else if ((bp.line != -1) && bp.verified ) //(ph 2025/01/27)
            {
                // match previous filePath and line numbers to update breakpoint
                int bpCount = GetBreakpointsCount();
                for (int ii=0; ii<bpCount; ++ii)
                {
                    cb::shared_ptr<cbBreakpoint> pBreakpoint = GetBreakpoint(ii);
                    wxString filePath = pBreakpoint->GetLocation();
                    // The filepath line number must match the DAP line number
                    if (filePath.Length() and (pBreakpoint->GetLine() == bp.line))
                        UpdateOrAddBreakpoint(filePath, bp.line, true, bp.id);
                }

            }//endelse

        }//endfor
    }//endif resp
}//end OnBreakpointDataSet

// "==================================================================================================================="
// "          ____       _      ____      _____  __     __  _____   _   _   _____   ____      _____   _   _   ____     "
// "         |  _ \     / \    |  _ \    | ____| \ \   / / | ____| | \ | | |_   _| / ___|    | ____| | \ | | |  _ \    "
// "         | | | |   / _ \   | |_) |   |  _|    \ \ / /  |  _|   |  \| |   | |   \___ \    |  _|   |  \| | | | | |   "
// "         | |_| |  / ___ \  |  __/    | |___    \ V /   | |___  | |\  |   | |    ___) |   | |___  | |\  | | |_| |   "
// "         |____/  /_/   \_\ |_|       |_____|    \_/    |_____| |_| \_|   |_|   |____/    |_____| |_| \_| |____/    "
// "                                                                                                                   "
// "==================================================================================================================="
