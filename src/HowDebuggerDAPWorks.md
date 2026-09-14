# How Debugger_DAP Plugin works

Based on the code analysis of the `src/DebugAdapterProtocol` directory, here is an explanation of how the Debug Adapter Protocol (DAP) implementation works in this project.

## Overview

This library provides a C++ implementation of the Microsoft Debug Adapter Protocol (DAP). It acts as a bridge (**Client**) between an IDE/Editor (the frontend) and a generic Debug Adapter (the backend, e.g., `lldb-vscode`, `debugpy`, or `gdb`).

The communication happens over a Socket connection using JSON-RPC messages.

## Key Components

1. **`dap` Namespace (Data Models)**
   
   * **Location:** `dap.hpp`, `dap.cpp`
   * **Role:** Defines the standard DAP messages as C++ structs.
   * **Structure:** All messages inherit from `ProtocolMessage`.
     * **Requests:** `InitializeRequest`, `LaunchRequest`, `SetBreakpointsRequest`.
     * **Responses:** `InitializeResponse`, `StackTraceResponse`.
     * **Events:** `StoppedEvent`, `OutputEvent`, `TerminatedEvent`.
   * **Serialization:** Macros like `JSON_SERIALIZE()` are used to convert these C++ structs to and from JSON format.

2. **`JsonRPC` (Transport Layer)**
   
   * **Location:** `JsonRPC.hpp`, `JsonRPC.cpp`
   * **Role:** Handles the low-level formatting and parsing of the protocol.
   * **Protocol Format:** Messages are sent with HTTP-style headers, specifically `Content-Length`, followed by `\r\n\r\n` and the JSON payload.
   * **Buffering:** It maintains an internal buffer (`m_buffer`) to handle fragmented network packets, extracting complete JSON messages once enough data is received.

3. **`Client` (The Controller)**
   
   * **Location:** `Client.hpp`, `Client.cpp`
   * **Role:** Manages the debug session and state.
   * **Threading:** It spawns a background Reader Thread (`StartReaderThread`) that continuously reads from the socket.
   * **Event Dispatch:** When a message arrives (like a `StoppedEvent`), the Client parses it and notifies the main application via wxWidgets events (e.g., `SendDAPEvent`) or callbacks.
   * **API:** Exposes high-level methods for the IDE to control the debugger:
     * `Initialize()`: Starts the handshake.
     * `SetBreakpointsFile()`: Maps source files to breakpoints.
     * `Launch()` / `Attach()`: Starts the debuggee.
     * `Next()`, `StepIn()`, `Continue()`: Controls execution.

## Typical Workflow

1. **Connection & Handshake:**
   
   * The Client connects via TCP to the Debug Adapter.
   * It sends an `InitializeRequest` to negotiate capabilities (e.g., "Do you support conditional breakpoints?").
   * The Adapter replies with an `InitializeResponse`.

2. **Configuration:**
   
   * The Client sends configuration requests like `SetBreakpointsRequest` or `SetExceptionBreakpointsRequest`.
   * Once finished, it calls `ConfigurationDone()`.

3. **Execution:**
   
   * The Client sends a `LaunchRequest` (to start a process) or `AttachRequest` (to join a running one).
   * The Adapter triggers a `StoppedEvent` (e.g., "entry").

4. **Debugging Loop:**
   
   * **User Action:** User clicks "Step Over".
   * **Client:** Sends `NextRequest`.
   * **Adapter:** Steps the thread, then sends `StoppedEvent` (reason: "step").
   * **Client:** Receives `StoppedEvent`, then automatically queries for state:
     * `GetThreads()` -> `ThreadsRequest`
     * `GetFrames()` -> `StackTraceRequest`
     * `GetScopes()` / `GetVariables()` -> Populates the "Locals" and "Watch" views.

### Code Snippet Example (Concept)

Here is how a request is constructed and sent in `Client.cpp` (conceptual):

```cpp
// 1. Create the Request Object
dap::NextRequest request;
request.arguments.threadId = m_active_thread_id;

// 2. Serialize and Send via JsonRPC
// The JsonRPC class wraps this in:
// Content-Length: <len>\r\n\r\n{"seq": 10, "type": "request", "command": "next", ...}
m_rpc.Send(request, m_transport);
```

---

## How to implement a new DAP call

To implement a new DAP call (e.g., `ReadMemory`) and receive a response, you need to touch the full stack from the protocol layer up to the plugin layer.

Here are the concrete code changes required.

### 1. Define Protocol Messages

**File:** `src/DebugAdapterProtocol/dap.hpp`

First, define the request and response structures inside the `dap` namespace.

```cpp
// In dap.hpp, inside namespace dap

struct WXDLLIMPEXP_DAP ReadMemoryArguments : public Any {
    wxString memoryReference;
    int offset = 0;
    int count = 0;
    ANY_CLASS(ReadMemoryArguments);
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP ReadMemoryRequest : public Request {
    ReadMemoryArguments arguments;
    REQUEST_CLASS(ReadMemoryRequest, "readMemory");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP ReadMemoryResponse : public Response {
    wxString address;
    int unreadableBytes = 0;
    wxString data; // base64 encoded bytes
    RESPONSE_CLASS(ReadMemoryResponse, "readMemory");
    JSON_SERIALIZE();
};
```

### 2. Implement Serialization

**File:** `src/DebugAdapterProtocol/dap.cpp`

Implement the `To()` and `From()` JSON methods and register the new classes in `Initialize()`.

```cpp
// In dap::Initialize()
REGISTER_CLASS(ReadMemoryRequest);
REGISTER_CLASS(ReadMemoryResponse);

// In dap.cpp (anywhere in the implementation section)

// --- ReadMemoryArguments ---
Json ReadMemoryArguments::To() const {
    CREATE_JSON();
    ADD_PROP(memoryReference);
    ADD_PROP(offset);
    ADD_PROP(count);
    return json;
}
void ReadMemoryArguments::From(const Json & json) {
    GET_PROP(memoryReference, String);
    GET_PROP(offset, Integer);
    GET_PROP(count, Integer);
}

// --- ReadMemoryRequest ---
Json ReadMemoryRequest::To() const {
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}
void ReadMemoryRequest::From(const Json & json) {
    REQUEST_FROM();
    READ_OBJ(arguments);
}

// --- ReadMemoryResponse ---
Json ReadMemoryResponse::To() const {
    RESPONSE_TO();
    ADD_BODY();
    ADD_BODY_PROP(address);
    ADD_BODY_PROP(unreadableBytes);
    ADD_BODY_PROP(data);
    return json;
}
void ReadMemoryResponse::From(const Json & json) {
    RESPONSE_FROM();
    READ_BODY();
    GET_BODY_PROP(address, String);
    GET_BODY_PROP(unreadableBytes, Integer);
    GET_BODY_PROP(data, String);
}
```

### 3. Define the Event

**File:** `src/DebugAdapterProtocol/DAPEvent.hpp` & `DAPEvent.cpp`

Create a new wxEvent to notify the plugin when the response arrives.

**In `DAPEvent.hpp`:**

```cpp
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_DAP, wxEVT_DAP_READ_MEMORY_RESPONSE, DAPEvent);
```

**In `DAPEvent.cpp`:**

```cpp
wxDEFINE_EVENT(wxEVT_DAP_READ_MEMORY_RESPONSE, DAPEvent);
```

### 4. Update the Transport Client

**File:** `src/DebugAdapterProtocol/Client.hpp` & `Client.cpp`

Add a method to trigger the request and handle the incoming response.

**In `Client.hpp`:**

```cpp
// Inside class Client public methods:
void ReadMemory(const wxString& memRef, int offset, int count);
```

**In `Client.cpp`:**

```cpp
// 1. Implement sending method
void dap::Client::ReadMemory(const wxString& memoryReference, int offset, int count)
{
    ReadMemoryRequest req = MakeRequest<ReadMemoryRequest>();
    req.arguments.memoryReference = memoryReference;
    req.arguments.offset = offset;
    req.arguments.count = count;
    SendRequest(req);
}

// 2. Update OnMessage to handle the response
void dap::Client::OnMessage(Json json)
{
    // ... existing code ...

    // inside the "else if (as_response)" block:
    else if (as_response->command == "readMemory")
    {
        SendDAPEvent(wxEVT_DAP_READ_MEMORY_RESPONSE, new dap::ReadMemoryResponse, json);
    }
}
```

### 5. Hook into the Plugin

**File:** `src/DebugAdapterClient/plugin.h` & `plugin.cpp`

Finally, use the new call in your plugin.

**In `plugin.h`:**

```cpp
// Inside Debugger_DAP class declaration
protected:
    void OnReadMemoryResponse(DAPEvent& event);
```

**In `plugin.cpp`:**

```cpp
// 1. Bind event in Constructor
Debugger_DAP::Debugger_DAP()
{
    // ... existing binds ...
    m_dapClient.Bind(wxEVT_DAP_READ_MEMORY_RESPONSE, &Debugger_DAP::OnReadMemoryResponse, this);
}

// 2. Unbind in Destructor
Debugger_DAP::~Debugger_DAP()
{
    // ... existing unbinds ...
    m_dapClient.Unbind(wxEVT_DAP_READ_MEMORY_RESPONSE, &Debugger_DAP::OnReadMemoryResponse, this);
}

// 3. Implement the Response Handler
void Debugger_DAP::OnReadMemoryResponse(DAPEvent & event)
{
    dap::ReadMemoryResponse* resp = event.GetDapResponse()->As<dap::ReadMemoryResponse>();
    if (resp && resp->success)
    {
        // 'resp->data' contains the base64 encoded memory content
        Manager::Get()->GetLogManager()->DebugLog("ReadMemory success. Data: " + resp->data);
    }
    else
    {
        Manager::Get()->GetLogManager()->DebugLog("ReadMemory failed: " + resp->message);
    }
}
```