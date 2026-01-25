/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
*/

#ifndef __DEBUGGER_STATE_H__
#define __DEBUGGER_STATE_H__

// ----------------------------------------------------------------------------
class Debugger_State
// ----------------------------------------------------------------------------
{
    public:
        enum eDAPState
        {
            NotConnected = 0x1,
            Connected    = 0x2,
            Stopped      = 0x4,
            Running      = 0x8
        };

        static bool IsRunning();
        static bool IsStopped();
        static bool IsBusy();
        static bool IsConnected();
        static void AddState(Debugger_State::eDAPState newState);
        static void RemoveState(Debugger_State::eDAPState stateToRemove);
        static bool HasState(Debugger_State::eDAPState state);

    private:
        static unsigned int DAPDebuggerState;
};

#endif // __DEBUGGER_STATE_H__
