/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filedlg.h>
#include <wx/intl.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>
#include <wx/socket.h> //(ph 2024/05/08)

#include <sdk.h>
#include "cbplugin.h"
#include "cbproject.h"
#include "compilerfactory.h"
#include "configmanager.h"
#include "logmanager.h"
#include "macrosmanager.h"
#include "globals.h"

#include "dlg_SettingsOptions.h"
#include "helpers.h" //(ph 2024/05/09)

namespace dbg_DAP
{
class DebuggerConfigurationPanel : public wxPanel
{
    public:
        void ValidateExecutablePath()
        {
            wxTextCtrl * pathCtrl = XRCCTRL(*this, "txtDAPExecutable", wxTextCtrl);
            wxString path = pathCtrl->GetValue();
            Manager::Get()->GetMacrosManager()->ReplaceEnvVars(path);

            if (not wxFileExists(path))
            {
                pathCtrl->SetForegroundColour(*wxWHITE);
                pathCtrl->SetBackgroundColour(*wxRED);
                pathCtrl->SetToolTip(_("Full path to the debugger's executable. Executable can't be found on the filesystem!"));
            }
            else
            {
                pathCtrl->SetForegroundColour(wxNullColour);
                pathCtrl->SetBackgroundColour(wxNullColour);
                pathCtrl->SetToolTip(_("Full path to the debugger's executable."));
            }

            pathCtrl->Refresh();
        }

    private:
        void OnBrowse(cb_unused wxCommandEvent & event)
        {
            wxString oldPath = XRCCTRL(*this, "txtDAPExecutable", wxTextCtrl)->GetValue();
            Manager::Get()->GetMacrosManager()->ReplaceEnvVars(oldPath);
            wxFileDialog dlg(this,
                             _("Select executable file"),
                             wxEmptyString,
                             oldPath,
                             wxFileSelectorDefaultWildcardStr,
                             wxFD_OPEN | wxFD_FILE_MUST_EXIST
#if defined(__WXMAC__) and  wxCHECK_VERSION(3,1,3)   // wxFD_SHOW_HIDDEN added in 3.1.3
                             | wxFD_SHOW_HIDDEN | compatibility::wxHideReadonly   // Needed to access /usr etc on MAC
#endif //WXMAC and //wx 3.1.3 or greater
                            );
            PlaceWindow(&dlg);

            if (dlg.ShowModal() == wxID_OK)
            {
                wxString newPath = dlg.GetPath();
                XRCCTRL(*this, "txtDAPExecutable", wxTextCtrl)->ChangeValue(newPath);
            }
        }

        void OnTextChange(cb_unused wxCommandEvent & event)
        {
            ValidateExecutablePath();
        }

    private:
        DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(DebuggerConfigurationPanel, wxPanel)
    EVT_BUTTON(XRCID("btnBrowse"), DebuggerConfigurationPanel::OnBrowse)
    EVT_TEXT(XRCID("txtDAPExecutable"), DebuggerConfigurationPanel::OnTextChange)
END_EVENT_TABLE()

DebuggerConfiguration::DebuggerConfiguration(const ConfigManagerWrapper & config) : cbDebuggerConfiguration(config)
{
}

cbDebuggerConfiguration * DebuggerConfiguration::Clone() const
{
    return new DebuggerConfiguration(*this);
}

wxPanel * DebuggerConfiguration::MakePanel(wxWindow * parent)
{
    DebuggerConfigurationPanel * panel = new DebuggerConfigurationPanel;

    if (not wxXmlResource::Get()->LoadPanel(panel, parent, "dlgDebuggerOptions_DAP"))
    {
        return panel;
    }

    XRCCTRL(*panel, "txtDAPExecutable", wxTextCtrl)->ChangeValue(GetDAP_Executable(false));
    panel->ValidateExecutablePath();
    XRCCTRL(*panel, "txtPortNumber",                    wxTextCtrl)->ChangeValue(GetDAP_PortNumber());
    XRCCTRL(*panel, "txtPythonHome",                    wxTextCtrl)->ChangeValue(GetDAP_PythonHomeEnvSetting());
    XRCCTRL(*panel, "txtInit",                          wxTextCtrl)->ChangeValue(GetInitialCommands());
    XRCCTRL(*panel, "txtInit",                          wxTextCtrl)->SetMinSize(wxSize(-1, 75));;

    XRCCTRL(*panel, "chkRunDAPServer",                  wxCheckBox)->SetValue(GetFlag(RunDAPServer));
    //-XRCCTRL(*panel, "chkStopOnMain",                    wxCheckBox)->SetValue(GetFlag(StopOnMain)); //(ph 2024/06/07)
    XRCCTRL(*panel, "chkExceptionCatch",                wxCheckBox)->SetValue(GetFlag(ExceptionCatch));
    XRCCTRL(*panel, "chkExceptionThrow",                wxCheckBox)->SetValue(GetFlag(ExceptionThrow));
    XRCCTRL(*panel, "chkPersistDebugElements",          wxCheckBox)->SetValue(GetFlag(PersistDebugElements));
    XRCCTRL(*panel, "choChoiceDebugTesting",            wxChoice)->SetSelection(m_config.ReadInt("debug_testing", 0));

    XRCCTRL(*panel, "chkTooltipEval",                   wxCheckBox)->SetValue(GetFlag(EvalExpression));
    XRCCTRL(*panel, "chkAddForeignDirs",                wxCheckBox)->SetValue(GetFlag(AddOtherProjectDirs));
    XRCCTRL(*panel, "chkWatchLocalsandArgs",            wxCheckBox)->SetValue(GetFlag(WatchFuncLocalsArgs));
    XRCCTRL(*panel, "choDisassemblyFlavor",             wxChoice)->SetSelection(m_config.ReadInt("disassembly_flavor", 0));
    XRCCTRL(*panel, "txtInstructionSet",                wxTextCtrl)->ChangeValue(m_config.Read("instruction_set", wxEmptyString));

    return panel;
}

bool DebuggerConfiguration::SaveChanges(wxPanel * panel)
{
    m_config.Write("executable_path",                   XRCCTRL(*panel, "txtDAPExecutable",         wxTextCtrl)->GetValue());
    m_config.Write("python_home_env",                   XRCCTRL(*panel, "txtPythonHome",            wxTextCtrl)->GetValue());
    m_config.Write("init_commands",                     XRCCTRL(*panel, "txtInit",                  wxTextCtrl)->GetValue());

    m_config.Write("run_DAP_server",                    XRCCTRL(*panel, "chkRunDAPServer",          wxCheckBox)->GetValue());
    //-m_config.Write("stop_on_main",                      XRCCTRL(*panel, "chkStopOnMain",            wxCheckBox)->GetValue());//(ph 2024/06/07)
    m_config.Write("exception_catch",                   XRCCTRL(*panel, "chkExceptionCatch",        wxCheckBox)->GetValue());
    m_config.Write("exception_throw",                   XRCCTRL(*panel, "chkExceptionThrow",        wxCheckBox)->GetValue());
    m_config.Write("persist_debug_elements",            XRCCTRL(*panel, "chkPersistDebugElements",  wxCheckBox)->GetValue());
    m_config.Write("debug_testing",                     XRCCTRL(*panel, "choChoiceDebugTesting",     wxChoice)->GetSelection());

    m_config.Write("watch_locals_and_args",             XRCCTRL(*panel, "chkWatchLocalsandArgs",    wxCheckBox)->GetValue());
    m_config.Write("eval_tooltip",                      XRCCTRL(*panel, "chkTooltipEval",           wxCheckBox)->GetValue());
    m_config.Write("add_other_search_dirs",             XRCCTRL(*panel, "chkAddForeignDirs",        wxCheckBox)->GetValue());
    m_config.Write("disassembly_flavor",                XRCCTRL(*panel, "choDisassemblyFlavor",     wxChoice)->GetSelection());
    m_config.Write("instruction_set",                   XRCCTRL(*panel, "txtInstructionSet",        wxTextCtrl)->GetValue());
    return true;
}

bool DebuggerConfiguration::GetFlag(eFlags flag)
{
    switch (flag)
    {
        case RunDAPServer:
            return m_config.ReadBool("run_DAP_server", true);

        case StopOnMain:
            return m_config.ReadBool("stop_on_main", false);

        case ExceptionCatch:
            return m_config.ReadBool("exception_catch", true);

        case ExceptionThrow:
            return m_config.ReadBool("exception_throw", true);

        case PersistDebugElements:
            return m_config.ReadBool("persist_debug_elements", false);



        case WatchFuncLocalsArgs:
            return m_config.ReadBool("watch_locals_and_args", true);

        case EvalExpression:
            return m_config.ReadBool("eval_tooltip", false);

        case AddOtherProjectDirs:
            return m_config.ReadBool("add_other_search_dirs", false);

        default:
            return false;
    }
}
void DebuggerConfiguration::SetFlag(eFlags flag, bool value)
{
    switch (flag)
    {
        case RunDAPServer:
            m_config.Write("run_DAP_server", value);

        case StopOnMain:
            m_config.Write("stop_on_main", value);

        case ExceptionCatch:
            m_config.Write("exception_catch", value);
            break;

        case ExceptionThrow:
            m_config.Write("exception_throw", value);
            break;

        case EvalExpression:
            m_config.Write("eval_tooltip", value);
           break;

        case PersistDebugElements:
            m_config.Write("persist_debug_elements", value);



        case WatchFuncLocalsArgs:
            m_config.Write("watch_locals_and_args", value);
            break;
        case AddOtherProjectDirs:
            m_config.Write("add_other_search_dirs", value);
            break;

        default:
            ;
    }
}

wxString DebuggerConfiguration::GetDAP_Executable(bool expandMacro)
{
    // LogManager *pLogMgr = Manager::Get()->GetLogManager();
    // pLogMgr->DebugLog(wxString::Format("DebuggerConfiguration::GetDAPExecutable : Line %d", __LINE__));
    wxString result = m_config.Read("executable_path", wxEmptyString);
    // pLogMgr->DebugLog(wxString::Format("DebuggerConfiguration::GetDAPExecutable : %s  (Line %d)", result, __LINE__));

    if (result.IsEmpty())
    {
        result = DetectDebuggerExecutable("lldb-dap");
    }
    else
    {
        if (expandMacro)
        {
            Manager::Get()->GetMacrosManager()->ReplaceEnvVars(result);
        }

        // pLogMgr->DebugLog(wxString::Format("GetDAPExecutable : %s  (Line %d)", result, __LINE__));
    }

    // pLogMgr->DebugLog(wxString::Format("GetDAPExecutable : %s  (Line %d)", result, __LINE__));

    if (result.IsEmpty() || (not wxFileExists(result)))
    {
        result = DetectDebuggerExecutable("lldb-dap");

        // pLogMgr->DebugLog(wxString::Format("GetDAPExecutable : %s  (Line %d)", result, __LINE__));
        if (result.IsEmpty())
        {
            if (platform::windows)
            {
                wxString fullFileName  = "C:\\msys64\\mingw64\\bin\\lldb-dap.exe";

                if (wxFileExists(fullFileName))
                {
                    result = fullFileName;
                }
                else
                {
                    fullFileName  = "C:\\msys64\\clang64\\bin\\lldb-dap.exe";

                    if (wxFileExists(fullFileName))
                    {
                        result = fullFileName;
                    }
                    else
                    {
                        fullFileName  = "C:\\msys64\\mingw32\\bin\\lldb-dap.exe";

                        if (wxFileExists(fullFileName))
                        {
                            result = fullFileName;
                        }
                        else
                        {
                            fullFileName  = "C:\\msys64\\clang32\\bin\\lldb-dap.exe";

                            if (wxFileExists(fullFileName))
                            {
                                result = fullFileName;
                            }
                            else
                            {
                                fullFileName  = "C:\\msys64";

                                if (wxDirExists(fullFileName))
                                {
                                    result = fullFileName;
                                }
                                else
                                {
                                    fullFileName = wxEmptyString;
                                }
                            }
                        }
                    }
                }
            }

            if (platform::Linux)
            {
                wxString fullFileName  = L"/usr/bin/lldb-vscode-14";

                if (wxFileExists(fullFileName))
                {
                    result = fullFileName;
                }
                else
                {
                    fullFileName  = L"/usr/bin/lldb-vscode-13";

                    if (wxFileExists(fullFileName))
                    {
                        result = fullFileName;
                    }
                    else
                    {
                        fullFileName  = L"/usr/bin/lldb-vscode-12";

                        if (wxFileExists(fullFileName))
                        {
                            result = fullFileName;
                        }
                        else
                        {
                            fullFileName  = L"/usr/bin/";

                            if (wxDirExists(fullFileName))
                            {
                                result = fullFileName;
                            }
                            else
                            {
                                fullFileName = wxEmptyString;
                            }
                        }
                    }
                }
            }

            if (platform::macosx)
            {
                // pLogMgr->DebugLog(wxString::Format("GetDAPExecutable : (Line %d)", __LINE__));
                wxString fullFileName  = "/usr/local/opt/llvm/bin/lldb-vscode";

                if (wxFileExists(fullFileName))
                {
                    result = fullFileName;
                    // pLogMgr->DebugLog(wxString::Format("GetDAPExecutable : %s  (Line %d)", result, __LINE__));
                }
                else
                {
                    fullFileName  = "/usr/local/Cellar/llvm";

                    if (wxDirExists(fullFileName))
                    {
                        result = fullFileName;
                        // pLogMgr->DebugLog(wxString::Format("GetDAPExecutable : %s  (Line %d)", result, __LINE__));
                    }
                    else
                    {
                        result = wxEmptyString;
                    }
                }
            }
        }
    }

    // pLogMgr->DebugLog(wxString::Format("GetDAPExecutable : %s  (Line %d)", result, __LINE__));
    return result;
}

//wxString DebuggerConfiguration::GetDAPPortNumber()
//{
//    wxString result = m_config.Read("port_number", "4711");
//
//    if (result.empty())
//    {
//        result = "4711";
//        //
//
//    }
//
//    return result;
//}

// ----------------------------------------------------------------------------
wxString DebuggerConfiguration::GetDAP_PortNumber()
// ----------------------------------------------------------------------------
{
    wxString resultStr = m_config.Read("port_number", "");

        return resultStr;
}

// ----------------------------------------------------------------------------
void DebuggerConfiguration::SetDAP_PortNumber(const wxString& port)
// ----------------------------------------------------------------------------
{
    m_config.Write("port_number", port);
}

wxString DebuggerConfiguration::GetDAP_PythonHomeEnvSetting()
{
    wxString result = m_config.Read("python_home_env", "");
    return result;
}

DebuggerConfiguration::eDebugTestingOptions DebuggerConfiguration::GetDebugTestingChoiceOption()
{
    return static_cast<eDebugTestingOptions>(m_config.ReadInt("debug_testing", 0));
}

wxString DebuggerConfiguration::GetDisassemblyFlavorCommand()
{
    int disassembly_flavour = m_config.ReadInt("disassembly_flavor", 0);
    wxString flavour = "set disassembly-flavor ";

    switch (disassembly_flavour)
    {
        case 1: // AT & T
        {
            flavour << "att";
            break;
        }

        case 2: // Intel
        {
            flavour << "intel";
            break;
        }

        case 3: // Custom
        {
            wxString instruction_set = m_config.Read("instruction_set", wxEmptyString);
            flavour << instruction_set;
            break;
        }

        default: // including case 0: // System default
            if (platform::windows)
            {
                flavour << "att";
            }
            else
            {
                flavour << "intel";
            }
    }// switch

    return flavour;
}

wxString DebuggerConfiguration::GetInitialCommands()
{
    return m_config.Read("init_commands", wxEmptyString);
}

wxString DebuggerConfiguration::SearchForDebuggerExecutable(wxString pathParam, const wxString & exeNameParam)
{
    LogManager * pLogMgr = Manager::Get()->GetLogManager();
    wxFileName fnDetectDebuggerExecutable;
    fnDetectDebuggerExecutable.SetFullName(exeNameParam);
    pathParam = pathParam.Trim().Trim(false);
    fnDetectDebuggerExecutable.SetPath(pathParam);

    if (fnDetectDebuggerExecutable.FileExists())
    {
        pLogMgr->DebugLog(wxString::Format(_("SearchForDebuggerExecutable detected %s"), fnDetectDebuggerExecutable.GetFullPath()));
        return fnDetectDebuggerExecutable.GetFullPath();
    }
    else
    {
        fnDetectDebuggerExecutable.SetPath(pathParam + wxFILE_SEP_PATH + "bin");

        if (fnDetectDebuggerExecutable.FileExists())
        {
            pLogMgr->DebugLog(wxString::Format(_("SearchForDebuggerExecutable detected %s"), fnDetectDebuggerExecutable.GetFullPath()));
            return fnDetectDebuggerExecutable.GetFullPath();
        }
    }

    return wxEmptyString;
}

wxString DebuggerConfiguration::DetectDebuggerExecutable(const wxString & exeNameParam)
{
    // LogManager *pLogMgr = Manager::Get()->GetLogManager();
    wxString masterPath = wxEmptyString;
    wxFileName exeName(exeNameParam);

    if (platform::windows)
    {
        if (exeName.GetExt().empty())
        {
            exeName.SetExt("exe");
        }
    }

    // Check Project default compiler path to see if file in it
    cbProject * pProject = Manager::Get()->GetProjectManager()->GetActiveProject();

    if (pProject)
    {
        // pLogMgr->DebugLog(wxString::Format("cbDetectDebuggerExecutable pProject found.(Line %d)", __LINE__));
        int compilerIdx = CompilerFactory::GetCompilerIndex(pProject->GetCompilerID());

        if (compilerIdx != wxNOT_FOUND)
        {
            Compiler * prjCompiler = CompilerFactory::GetCompiler(compilerIdx);

            if (prjCompiler)
            {
                masterPath = prjCompiler->GetMasterPath();

                if (not masterPath.IsEmpty())
                {
                    wxString debuggerSearchResult = SearchForDebuggerExecutable(masterPath, exeName.GetFullName());

                    if (not debuggerSearchResult.IsEmpty())
                    {
                        // pLogMgr->DebugLog(wxString::Format("cbDetectDebuggerExecutable : debugger %s (Line %d)", debuggerSearchResult, __LINE__));
                        return debuggerSearchResult;
                    }
                }
            }
        }
    }

    // else
    // {
    //    pLogMgr->DebugLog(wxString::Format("cbDetectDebuggerExecutable no project found (Line %d)",  __LINE__));
    // }
    // Check default compiler path to see if file in it
    Compiler * defaultCompiler = CompilerFactory::GetDefaultCompiler();

    if (defaultCompiler)
    {
        masterPath = defaultCompiler->GetMasterPath();

        // pLogMgr->DebugLog(wxString::Format("cbDetectDebuggerExecutable defaultCompiler found. masterPath %s  (Line %d)", masterPath, __LINE__));
        if ((not masterPath.IsEmpty()) && wxDirExists(masterPath + wxFILE_SEP_PATH + "bin"))
        {
            wxString debuggerSearchResult = SearchForDebuggerExecutable(masterPath, exeName.GetFullName());

            if (not debuggerSearchResult.IsEmpty())
            {
                // pLogMgr->DebugLog(wxString::Format("cbDetectDebuggerExecutable : debugger %s (Line %d)", debuggerSearchResult, __LINE__));
                return debuggerSearchResult;
            }
        }
    }

    // else
    // {
    //     pLogMgr->DebugLog(wxString::Format("cbDetectDebuggerExecutable no defaultCompiler found (Line %d)",  __LINE__));
    // }
    wxString exePath = cbFindFileInPATH(exeName.GetFullName());

    if (not wxDirExists(exePath))
    {
        return wxEmptyString;
    }

    return exePath + wxFILE_SEP_PATH + exeName.GetFullName();
}

} // namespace dbg_DAP

