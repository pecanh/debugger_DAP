#include "DAP_Debugger_State.h"

//#include <bitset>

unsigned int Debugger_State::DAPDebuggerState = Debugger_State::eDAPState::NotConnected;

// ----------------------------------------------------------------------------
bool Debugger_State::IsRunning()
// ----------------------------------------------------------------------------
{
    return (Debugger_State::DAPDebuggerState & Debugger_State::eDAPState::Running) != 0;
}

// ----------------------------------------------------------------------------
bool Debugger_State::IsStopped()
// ----------------------------------------------------------------------------
{
    return (Debugger_State::DAPDebuggerState & Debugger_State::eDAPState::Stopped) != 0;
}
// ----------------------------------------------------------------------------
bool Debugger_State::IsConnected()
// ----------------------------------------------------------------------------
{
    return (Debugger_State::DAPDebuggerState & Debugger_State::eDAPState::Connected) != 0;
}
// ----------------------------------------------------------------------------
bool Debugger_State::IsBusy()
// ----------------------------------------------------------------------------
{
    return ( (Debugger_State::DAPDebuggerState & Debugger_State::eDAPState::Connected) &&
             (Debugger_State::DAPDebuggerState & Debugger_State::eDAPState::Running) &&
             ( not (Debugger_State::DAPDebuggerState & Debugger_State::eDAPState::Stopped)) );
}
// ----------------------------------------------------------------------------
void Debugger_State::AddState(Debugger_State::eDAPState newState)
// ----------------------------------------------------------------------------
{
    Debugger_State::DAPDebuggerState |= newState;
    if (IsConnected() and Debugger_State::eDAPState::NotConnected)
        RemoveState(Debugger_State::eDAPState::NotConnected);
    if (not IsConnected() )
    {
        RemoveState(Debugger_State::eDAPState::Connected);
        RemoveState(Debugger_State::eDAPState::Running);
        Debugger_State::DAPDebuggerState |= Stopped;
    }
    if (IsBusy()) RemoveState(Debugger_State::eDAPState::Stopped);
}
// ----------------------------------------------------------------------------
void Debugger_State::RemoveState(Debugger_State::eDAPState stateToRemove)
// ----------------------------------------------------------------------------
{
    Debugger_State::DAPDebuggerState &= ~stateToRemove;
    if (stateToRemove == Debugger_State::eDAPState::Connected)
    {
        // Also remove Running and set Stopped;
        Debugger_State::DAPDebuggerState &= ~Debugger_State::eDAPState::Running;
        Debugger_State::DAPDebuggerState |= Stopped;
    }
}

// ----------------------------------------------------------------------------
bool Debugger_State::HasState(Debugger_State::eDAPState state)
// ----------------------------------------------------------------------------
{
    return (Debugger_State::DAPDebuggerState & state) != 0;
}
