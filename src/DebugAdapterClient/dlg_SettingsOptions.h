/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef __DEBUGGER_DAP_DLG_SETTINGSOPTIONS_H__
#define __DEBUGGER_DAP_DLG_SETTINGSOPTIONS_H__

#include <debuggermanager.h>

class ConfigManagerWrapper;

namespace dbg_DAP
{
class DebuggerConfiguration : public cbDebuggerConfiguration
{
    public:
        explicit DebuggerConfiguration(const ConfigManagerWrapper & config);

        cbDebuggerConfiguration * Clone() const override;
        wxPanel * MakePanel(wxWindow * parent) override;
        bool SaveChanges(wxPanel * panel) override;

    public:
        typedef enum
        {
            RunDAPServer = 0,
            StopOnMain,
            ExceptionCatch,
            ExceptionThrow,
            PersistDebugElements,

            WatchFuncLocalsArgs,
            EvalExpression,
            AddOtherProjectDirs
        } eFlags;

        typedef enum
        {
            UseNativePaths = 0,
            UseLinuxPathsOnWindowsWithDriveLetter,
            UseLinuxPathsOnWindowsWithNoCollen,
            UseLinuxPathsOnWindowsWithNoDriveLetter,
            UseWindowsPathsWithDriveLetterRemoved,
            UseRelativePathComparedToTheExecutable
        } eDebugTestingOptions;

        bool GetFlag(eFlags flag);
        void SetFlag(eFlags flag, bool value);
        wxString GetDAP_Executable(bool expandMacro = true);
        wxString GetDAP_PortNumber();
        void SetDAP_PortNumber(const wxString& port);
        wxString GetDAP_PythonHomeEnvSetting();
        eDebugTestingOptions GetDebugTestingChoiceOption();
        wxString GetDisassemblyFlavorCommand();
        wxString GetInitialCommands();


    private:
        wxString SearchForDebuggerExecutable(wxString pathParam, const wxString & exeNameParam);
        wxString DetectDebuggerExecutable(const wxString & exeNameParam);

};

} // namespace dbg_DAP

#endif // __DEBUGGER_DAP_DLG_SETTINGSOPTIONS_H__
