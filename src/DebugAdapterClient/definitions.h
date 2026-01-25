/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
*/

#ifndef _DEBUGGER_DAP_DEFINITIONS_H_
#define _DEBUGGER_DAP_DEFINITIONS_H_

// System and library includes
#include <deque>
#include <unordered_map>
#include <tinyxml2.h>
//-#include <memory>
#include <wx/sizer.h>
#include <wx/string.h>
#include <scrollingdialog.h>

// CB includes
#include <debuggermanager.h>
#include <cbplugin.h> //(ph 2024/08/03)

// DAP include files
#include "debugger_logger.h"

class Debugger_DAP;

namespace dbg_DAP
{
wxString BoolTowxString(bool value);
bool wxStringToBool(wxString value);

void AddChildNode(tinyxml2::XMLNode * pNodeParent,  const wxString name, const wxString value);
void AddChildNode(tinyxml2::XMLNode * pNodeParent,  const wxString name, const int iValue);
void AddChildNode(tinyxml2::XMLNode * pNodeParent,  const wxString name, const long lValue);
void AddChildNode(tinyxml2::XMLNode * pNodeParent,  const wxString name, const uint64_t lValue);
void AddChildNodeHex(tinyxml2::XMLNode * pNodeParent,  const wxString name, const uint64_t llValue);
void AddChildNode(tinyxml2::XMLNode * pNodeParent,  const wxString name, const bool bValue);

wxString ReadChildNodewxString(tinyxml2::XMLElement * pElementParent,  const wxString childName);
int ReadChildNodeInt(tinyxml2::XMLElement * pElementParent,  const wxString childName);
long ReadChildNodeLong(tinyxml2::XMLElement * pElementParent,  const wxString childName);
uint64_t ReadChildNodeUint64(tinyxml2::XMLElement * pElementParent,  const wxString childName);
uint64_t ReadChildNodeHex(tinyxml2::XMLElement * pElementParent,  const wxString childName);
bool ReadChildNodeBool(tinyxml2::XMLElement * pElementParent,  const wxString childName);

enum ResetDataType
{
    ResetData_Normal = 0,
    ResetData_Startup,
    ResetData_All
};

// ----------------------------------------------------------------------------
class DAPBreakpoint : public cbBreakpoint
// ----------------------------------------------------------------------------
{
    public:
        enum BreakpointType
        {
            bptCode = 0,    // Normal file/line breakpoint
            bptFunction,    // Function signature breakpoint
            bptData         // Data breakpoint
        };

        DAPBreakpoint(cbProject * project, dbg_DAP::LogPaneLogger * logger) :
            m_pLogger(logger),
            m_type(bptCode),
            m_project(project),
            m_filename(wxEmptyString),
            m_line(-1),
            m_ID(-1),
            m_temporary(false),
            m_enabled(true),
            m_active(true),
            m_useIgnoreCount(false),
            m_ignoreCount(0),
            m_useCondition(false),
            m_wantsCondition(false),
            m_address(0),
            m_alreadySet(false),
            m_breakOnRead(false),
            m_breakOnWrite(true)
        {
        }

        DAPBreakpoint(cbProject * project, dbg_DAP::LogPaneLogger * logger, const wxString & filename, int line, int id) :
            m_pLogger(logger),
            m_type(bptCode),
            m_project(project),
            m_filename(filename),
            m_line(line),
            m_ID(id),
            m_temporary(false),
            m_enabled(true),
            m_active(true),
            m_useIgnoreCount(false),
            m_ignoreCount(0),
            m_useCondition(false),
            m_wantsCondition(false),
            m_address(0),
            m_alreadySet(false),
            m_breakOnRead(false),
            m_breakOnWrite(true)
        {
        }

        // from cbBreakpoint
        virtual void SetEnabled(bool flag);
        virtual wxString GetLocation() const;
        virtual int GetLine() const;
        virtual wxString GetLineString() const;
        virtual wxString GetType() const;
        virtual wxString GetInfo() const;
        virtual bool IsEnabled() const;
        virtual bool IsVisibleInEditor() const;
        virtual bool IsTemporary() const;

        // DAP additional
        const wxString & GetCondition() const
        {
            return m_condition;
        }

        int GetIgnoreCount() const
        {
            return 0;
        }

        bool HasCondition() const
        {
            return false;
        }

        bool HasIgnoreCount() const
        {
            return false;
        }

        void SetShiftLines(int linesToShift)
        {
            m_line += linesToShift;
        }

        cbProject * GetProject()
        {
            return m_project;
        }

        wxString GetTypewxString()
        {
            switch (m_type)
            {
                case BreakpointType::bptCode:
                    return "bptCode";

                case BreakpointType::bptFunction:
                    return "bptFunction";

                case BreakpointType::bptData:
                    return "bptData";

                default:
                    return "unknown";
            }
        }

        BreakpointType GetType()
        {
            return m_type;
        }

        void SetType(wxString type)
        {
            if (type.IsSameAs("bptCode", false))
            {
                m_type = BreakpointType::bptCode;
                return;
            }

            if (type.IsSameAs("bptFunction", false))
            {
                m_type = BreakpointType::bptFunction;
                return;
            }

            if (type.IsSameAs("bptData", false))
            {
                m_type = BreakpointType::bptData;
                return;
            }
        }

        void SetType(BreakpointType type)
        {
            m_type = type;
        }

        wxString GetFilename()
        {
            return m_filename;
        }

        void SetFilename(wxString filename)
        {
            m_filename = filename;
        }

        int GetLine()
        {
            return m_line;
        }
        void SetLine(int line)
        {
            m_line = line;
        }

        int GetID()
        {
            return m_ID;
        }
        void SetID(int id)
        {
            m_ID = id;
        }

        bool GetIsTemporary()
        {
            return m_temporary;
        }

        void SetIsTemporary(bool temporary)
        {
            m_temporary = temporary;
        }

        bool GetIsEnabled()
        {
            return m_enabled;
        }

        void SetIsEnabled(bool enabled)
        {
            m_enabled = enabled;
        }

        bool GetIsActive()
        {
            return m_active;
        }

        void SetIsActive(bool active)
        {
            m_active = active;
        }


        bool GetIsUseIgnoreCount()
        {
            return m_useIgnoreCount;
        }

        void SetIsUseIgnoreCount(bool useIgnoreCount)
        {
            m_useIgnoreCount = useIgnoreCount;
        }

        int GetIgnoreCount()
        {
            return m_ignoreCount;
        }

        void SetIgnoreCount(int ignoreCount)
        {
            m_ignoreCount = ignoreCount;
        }

        bool GetIsUseCondition()
        {
            return m_useCondition;
        }

        void SetIsUseCondition(bool useCondition)
        {
            m_useCondition = useCondition;
        }

        bool GetIsWantsCondition()
        {
            return m_wantsCondition;
        }

        void SetIsWantsCondition(bool wantsCondition)
        {
            m_wantsCondition = wantsCondition;
        }

        wxString GetCondition()
        {
            return m_condition;
        }

        void SetCondition(wxString condition)
        {
            m_condition = condition;
        }

        wxString GetFunction()
        {
            return m_function;
        }
        void SetFunction(wxString function)
        {
            m_function = function;
        }

        uint64_t GetAddress()
        {
            return m_address;
        }

        void SetAddress(uint64_t address)
        {
            m_address = address;
        }

        bool GetisAlreadySet()
        {
            return m_alreadySet;
        }
        void SetIsAlreadySet(bool alreadySet)
        {
            m_alreadySet = alreadySet;
        }

        wxString GetLineText()
        {
            return m_lineText;
        }
        void SetLineText(wxString lineText)
        {
            m_lineText = lineText;
        }

        wxString GetBreakAddress()
        {
            return m_breakAddress;
        }

        void SetBreakAddress(wxString breakAddress)
        {
            m_breakAddress = breakAddress;
        }

        bool GetIsBreakOnRead()
        {
            return m_breakOnRead;
        }

        void SetIsBreakOnRead(bool breakOnRead)
        {
            m_breakOnRead = breakOnRead;
        }

        bool GetIsBreakOnWrite()
        {
            return m_breakOnWrite;
        }

        void SetIsBreakOnWrite(bool breakOnWrite)
        {
            m_breakOnWrite = breakOnWrite;
        }

        // DAP additional
        void SaveBreakpointToXML(tinyxml2::XMLNode * pNodeParent);
        void LoadBreakpointFromXML(tinyxml2::XMLElement * pElementBreakpoint, Debugger_DAP * dbgDAP);

    private:
        dbg_DAP::LogPaneLogger * m_pLogger;

        BreakpointType m_type;          // The type of this breakpoint.

        cbProject * m_project;          // The Project the file belongs to.
        wxString m_filename;            // The filename for the breakpoint.
        int m_line;                     // The line for the breakpoint.
        int m_ID;                       // DAP ID for the breakpoint
        //wxString filenameAsPassed;    // The filename for the breakpoint as passed to the debugger (i.e. full filename).

        bool m_temporary;               // Is this a temporary (one-shot) breakpoint?
        bool m_enabled;                 // Is the breakpoint enabled?
        bool m_active;                  // Is the breakpoint active? (currently unused)
        bool m_useIgnoreCount;          // Should this breakpoint be ignored for the first X passes? (@c x == @c ignoreCount)
        int m_ignoreCount;              // The number of passes before this breakpoint should hit. @c useIgnoreCount must be true.
        bool m_useCondition;            // Should this breakpoint hit only if a specific condition is met?
        bool m_wantsCondition;          // Evaluate condition for pending breakpoints at first stop !
        wxString m_condition;           // The condition that must be met for the breakpoint to hit. @c useCondition must be true.
        wxString m_function;            // The function to set the breakpoint. If this is set, it is preferred over the filename/line combination.
        uint64_t m_address;             // The actual breakpoint address. This is read back from the debugger. *Don't* write to it.
        bool m_alreadySet;              // Is this already set? Used to mark temporary breakpoints for removal.
        wxString m_lineText;            // Optionally, the breakpoint line's text (used by DAP for setting breapoints on ctors/dtors).
        wxString m_breakAddress;        // Valid only for type==bptData: address to break when read/written.
        bool m_breakOnRead;             // Valid only for type==bptData: break when memory is read from.
        bool m_breakOnWrite;            // Valid only for type==bptData: break when memory is written to.
};
// ----------------------------------------------------------------------------
class cbStackFrameXtn : public cbStackFrame     //(ph 2024/08/29)
// ----------------------------------------------------------------------------
{
    public:
        typedef cb::shared_ptr<cbStackFrameXtn> Pointer;
        typedef cb::shared_ptr<const cbStackFrameXtn> ConstPointer;

    public:
        cbStackFrameXtn(){::cbStackFrame();}
        int  GetDAPStackId() const { return m_DAPStackId; }
        void SetDAPStackId(int stackId){m_DAPStackId = stackId;}

    private:
        int m_DAPStackId;

};

typedef std::vector<cb::shared_ptr<dbg_DAP::DAPBreakpoint> > DAPBreakpointsContainer;
// Use the cbStackFrameXtn, not the cbStackFrame class in order to save the 'DAP' stack id.
typedef std::deque<cb::shared_ptr<cbStackFrameXtn> > DAPBacktraceContainer; //(ph 2024/08/29)
typedef std::deque<cb::shared_ptr<cbThread> > DAPThreadsContainer;

// ----------------------------------------------------------------------------
class DAPWatch : public cbWatch
// ----------------------------------------------------------------------------
{
    public:
        /** Watch variable format.
          *
          * @note not all formats are implemented for all debugger drivers.
          */
        enum WatchFormat
        {
            Undefined = 0,  // Format is undefined (whatever the debugger uses by default).
            Decimal,        // Variable should be displayed as decimal.
            Unsigned,       // Variable should be displayed as unsigned.
            Hex,            // Variable should be displayed as hexadecimal (e.g. 0xFFFFFFFF).
            Binary,         // Variable should be displayed as binary (e.g. 00011001).
            Char,           // Variable should be displayed as a single character (e.g. 'x').
            Float,          // Variable should be displayed as floating point number (e.g. 14.35)

            // do not remove these
            Last,           // used for iterations
            Any             // used for watches searches
        };

    public:

        DAPWatch(cbProject * project, dbg_DAP::LogPaneLogger * logger, wxString const & symbol, bool for_tooltip, bool delete_on_collapse = true) :
            m_project(project),
            m_pLogger(logger),
            m_DAPWatchClassName("DAPWatch"),
            m_DAPVariableReference(0),
            m_DAPChildVariableRequestSequence(0),
            m_symbol(symbol),
            m_address(0),
            m_type(wxEmptyString),
            m_format(WatchFormat::Undefined),
            m_debug_string(wxEmptyString),
            m_has_been_expanded(false),
            m_for_tooltip(for_tooltip),
            m_delete_on_collapse(delete_on_collapse),
            m_array_start(-1),
            m_array_end(-1),
            m_is_array(false),
            m_forTooltip(false),
            m_value(wxEmptyString),
            m_ValueErrorMessage(false)
        {
        }

        dbg_DAP::LogPaneLogger * GetDAPLogger()
        {
            return m_pLogger;
        }

        void Reset()
        {
            m_DAPVariableReference = 0;
            m_DAPChildVariableRequestSequence = 0,
            m_type = m_value = wxEmptyString;
            m_ValueErrorMessage = false;
            m_has_been_expanded = false;
            RemoveChildren();
            m_array_start = -1;
            m_array_end = -1;
            Expand(false);
        }

        void SetRangeArray(long iStart, long iEnd)
        {
            m_array_start = iStart;
            m_array_end = iEnd;
        }

        long GetRangeArrayStart()
        {
            return m_array_start;
        }

        long GetRangeArrayEnd()
        {
            return m_array_end;
        }

        void SetIsArray(bool isArray)
        {
            m_is_array = isArray;
        }

        bool GetIsArray()
        {
            return m_is_array;
        }

        void SetForTooltip(bool bForTooltip)
        {
            m_forTooltip = bForTooltip;
        }

        bool GetForTooltip()
        {
            return m_forTooltip;
        }

        uint64_t const GetDAPVariableReference() const
        {
            return m_DAPVariableReference;
        }

        void SetDAPChildVariableRequestSequence(int id)
        {
            m_DAPChildVariableRequestSequence = id;
        }

        int const GetDAPChildVariableRequestSequence() const
        {
            return m_DAPChildVariableRequestSequence;
        }

        void SetDAPVariableReference(uint64_t const & id)
        {
            m_DAPVariableReference = id;
        }

        bool HasBeenExpanded() const
        {
            return m_has_been_expanded;
        }

        void SetHasBeenExpanded(bool expanded)
        {
            m_has_been_expanded = expanded;
        }

        bool ForTooltip() const
        {
            return m_for_tooltip;
        }

        void SetDeleteOnCollapse(bool delete_on_collapse)
        {
            m_delete_on_collapse = delete_on_collapse;
        }

        bool DeleteOnCollapse() const
        {
            return m_delete_on_collapse;
        }

        void GetSymbol(wxString & symbol) const  override
        {
            symbol = m_symbol;
        }

        wxString GetSymbol() const
        {
            return m_symbol;
        }

        void SetSymbol(const wxString & symbol) override
        {
            m_symbol = symbol;
        }

        uint64_t GetAddress() const override
        {
            return m_address;
        }

        void SetAddress(uint64_t address) override
        {
            m_address = address;
        }

        void GetValue(wxString & value) const  override
        {
            value = m_value;
        }

        bool SetValue(const wxString & value) override
        {
            m_value = value;
            return true;
        }

        bool GetIsValueErrorMessage() override
        {
            return m_ValueErrorMessage;
        }

        void SetIsValueErrorMessage(bool value) override
        {
            m_ValueErrorMessage = value;
        }

        void GetFullWatchString(wxString & full_watch) const override
        {
            full_watch = m_value;
        }

        void GetType(wxString & type) const override
        {
            type = m_type;
        }

        void SetType(const wxString & type) override
        {
            m_type = type;
        }

        void SetFormat(WatchFormat format)
        {
            m_format = format;
        }

        void SetFormat(wxString wFormat)
        {
            m_format = GetWatchFormatFromwxString(wFormat);
        }

        WatchFormat GetFormat() const
        {
            return m_format;
        }
        wxString GetWatchFormatTowxString();
        WatchFormat GetWatchFormatFromwxString(wxString wFormat);

        wxString GetDebugString() const override
        {
            //-m_debug_string = m_DAPVariableReference + "->" + m_symbol + " = " + m_value;
            m_debug_string = wxString::Format("%llu", m_DAPVariableReference) + "->" + m_symbol + " = " + m_value; //(ph 2024/06/17)
            return m_debug_string;
        }

        cbProject * GetProject()
        {
            return m_project;
        }

        wxString MakeSymbolToAddress() const override
        {
            return wxT("&") + m_symbol;
        }

        // DAP additional
        void SaveWatchToXML(tinyxml2::XMLNode * pWatchesMasterNode);
        void LoadWatchFromXML(tinyxml2::XMLElement * pElementWatch);

    protected:
        virtual void DoDestroy() {}

    private:
        cbProject * m_project;              // The Project the watch belongs to.
        dbg_DAP::LogPaneLogger * m_pLogger;

        wxString m_DAPWatchClassName;
        uint64_t m_DAPVariableReference;
        int m_DAPChildVariableRequestSequence;
        wxString m_symbol;
        uint64_t m_address;
        wxString m_type;
        WatchFormat m_format;

        mutable wxString m_debug_string;
        bool m_has_been_expanded;
        bool m_for_tooltip;
        bool m_delete_on_collapse;

        long m_array_start;
        long m_array_end;
        bool m_is_array;
        bool m_forTooltip;

        wxString m_value;
        bool m_ValueErrorMessage;   // True if the m_value is a message instead of data
};

typedef std::vector<cb::shared_ptr<DAPWatch>> DAPWatchesContainer;

// cb::shared_ptr<DAPWatch> FindWatch(wxString const & expression, DAPWatchesContainer & watches);

// ----------------------------------------------------------------------------
class DAPMemoryRangeWatch  : public cbWatch
// ----------------------------------------------------------------------------
{
    public:
        DAPMemoryRangeWatch(cbProject * project, dbg_DAP::LogPaneLogger * logger, uint64_t address, uint64_t size, const wxString & symbol);

    public:
        void GetSymbol(wxString & symbol) const override
        {
            symbol = m_symbol;
        }

        wxString GetSymbol()
        {
            return m_symbol;
        }

        void SetSymbol(const wxString & symbol) override
        {
            m_symbol = symbol;
        }

        uint64_t GetAddress() const  override
        {
            return m_address;
        }
        void SetAddress(uint64_t address) override
        {
            m_address = address;
        }

        void GetValue(wxString & value) const override
        {
            value = m_value;
        }

        bool SetValue(const wxString & value) override;

        bool GetIsValueErrorMessage()  override
        {
            return m_ValueErrorMessage;
        }

        void SetIsValueErrorMessage(bool value)  override
        {
            m_ValueErrorMessage = value;
        }

        void GetFullWatchString(wxString & full_watch) const override
        {
            full_watch = wxEmptyString;
        }

        void GetType(wxString & type) const override
        {
            type = wxT("Memory range");
        }

        void SetType(cb_unused const wxString & type) override
        {
        }

        wxString GetDebugString() const override
        {
            return wxString();
        }

        wxString MakeSymbolToAddress() const override;
        bool IsPointerType() const override
        {
            return false;
        }

        uint64_t GetSize() const
        {
            return m_size;
        }

        cbProject * GetProject()
        {
            return m_project;
        }

        // DAP additional
        void SaveWatchToXML(tinyxml2::XMLNode * pWatchesMasterNode);
        void LoadWatchFromXML(tinyxml2::XMLElement * pElementWatch, Debugger_DAP * dbgDAP);

    private:
        cbProject * m_project;              // The Project the watch belongs to.
        dbg_DAP::LogPaneLogger * m_pLogger;
        wxString m_DAPWatchClassName;

        uint64_t m_address;
        uint64_t m_size;
        wxString m_symbol;
        wxString m_value;

        bool m_ValueErrorMessage;
};

typedef std::vector<cb::shared_ptr<DAPWatch>> DAPWatchesContainer;
typedef std::vector<cb::shared_ptr<DAPMemoryRangeWatch>> DAPMemoryRangeWatchesContainer;

// ----------------------------------------------------------------------------
enum class DAPWatchType
// ----------------------------------------------------------------------------
{
    Normal,
    MemoryRange
};

typedef std::unordered_map<cb::shared_ptr<cbWatch>, DAPWatchType> DAPMapWatchesToType;

// Custom window to display output of DebuggerInfoCmd
// ----------------------------------------------------------------------------
class DAPTextInfoWindow : public wxScrollingDialog
// ----------------------------------------------------------------------------
{
    public:
        DAPTextInfoWindow(wxWindow * parent, const wxChar * title, const wxString & content) :
            wxScrollingDialog(parent, -1, title, wxDefaultPosition, wxDefaultSize,
                              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX),
            m_font(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL)
        {
            wxSizer * sizer = new wxBoxSizer(wxVERTICAL);
            m_text = new wxTextCtrl(this, -1, content, wxDefaultPosition, wxDefaultSize,
                                    wxTE_READONLY | wxTE_MULTILINE | wxTE_RICH2 | wxHSCROLL);
            m_text->SetFont(m_font);
            sizer->Add(m_text, 1, wxGROW);
            SetSizer(sizer);
            sizer->Layout();
        }
        void SetText(const wxString & text)
        {
            m_text->SetValue(text);
            m_text->SetFont(m_font);
        }
    private:
        wxTextCtrl * m_text;
        wxFont m_font;
};

// ----------------------------------------------------------------------------
class DAPCurrentFrame
// ----------------------------------------------------------------------------
{
    public:
        DAPCurrentFrame() :
            m_line(-1),
            m_stack_frame(-1),
            m_user_selected_stack_frame(-1),
            m_thread(-1)
        {
        }

        void Reset()
        {
            m_stack_frame = -1;
            m_user_selected_stack_frame = -1;
        }

        void DAPSwitchToFrame(int frame_number)
        {
            m_user_selected_stack_frame = m_stack_frame = frame_number;
        }

        void SetFrame(int frame_number)
        {
            if (m_user_selected_stack_frame >= 0)
            {
                m_stack_frame = m_user_selected_stack_frame;
            }
            else
            {
                m_stack_frame = frame_number;
            }
        }
        void SetThreadId(int thread_id)
        {
            m_thread = thread_id;
        }
        void SetPosition(wxString const & filename, int line)
        {
            m_filename = filename;
            m_line = line;
        }

        int GetStackFrame() const
        {
            return m_stack_frame;
        }
        int GetUserSelectedFrame() const
        {
            return m_user_selected_stack_frame;
        }
        void GetPosition(wxString & filename, int & line)
        {
            filename = m_filename;
            line = m_line;
        }
        int GetThreadId() const
        {
            return m_thread;
        }

    private:
        wxString m_filename;
        int m_line;
        int m_stack_frame;
        int m_user_selected_stack_frame;
        int m_thread;
};

// Use this function to sanitize user input which might end as the last part of DAP commands.
// If the last character is '\', DAP will treat it as line continuation and it will stall.
wxString CleanStringValue(wxString value);

} // namespace dbg_DAP


// This class allows us to refer to the cbDebbuggerPlugin 'enum StartType' declaration
// without modifying the cbDebuggerPlugin class in cbplugin.h //(ph 2024/08/03)
// ----------------------------------------------------------------------------
class cbDebuggerPluginExt : public cbDebuggerPlugin
// ----------------------------------------------------------------------------
{
    //Helper class to access debugger StartType
   friend class Debugger_DAP;
   friend class DBG_DAP_Breakpoints;
   friend class DBG_DAP_CallStack;
   public:
    cbDebuggerPluginExt() : cbDebuggerPlugin("DAP", "debugger_dap" ) {};
    ~cbDebuggerPluginExt(){};
};

#endif // _DEBUGGER_DAP_DEFINITIONS_H_
