##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=dapcxx
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd
ProjectPath            :=D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap
IntermediateDirectory  :=../build-$(WorkspaceConfiguration)/dap
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=andrew
Date                   :=21/06/2022
CodeLitePath           :="C:/Program Files/CodeLite"
MakeDirCommand         :=mkdir
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputDirectory        :=D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/build-$(WorkspaceConfiguration)/lib
OutputFile             :=..\build-$(WorkspaceConfiguration)\lib\lib$(ProjectName).dll
Preprocessors          :=$(PreprocessorSwitch)WXMAKINGDLL_DAP 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
RcCmpOptions           := 
RcCompilerName         :=windres
LinkOptions            :=  $(shell wx-config --libs)
IncludePath            := $(IncludeSwitch)C:\msys64\mingw64\include\wx-3.1  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)ws2_32 
ArLibs                 :=  "ws2_32" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overridden using an environment variable
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -g -std=c++11 $(shell wx-config --cflags) -O0 $(Preprocessors)
CFLAGS   :=  -g $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
PATH:=C:\msys64\mingw64\bin;$PATH
WXWIN:=D:\Andrew_Development\Libraries\wxWidgets-3.1.7_win64
WXCFG:=gcc_dll\mswu
Objects0=$(IntermediateDirectory)/cJSON.cpp$(ObjectSuffix) $(IntermediateDirectory)/dap.cpp$(ObjectSuffix) $(IntermediateDirectory)/StringUtils.cpp$(ObjectSuffix) $(IntermediateDirectory)/Client.cpp$(ObjectSuffix) $(IntermediateDirectory)/Socket.cpp$(ObjectSuffix) $(IntermediateDirectory)/JSON.cpp$(ObjectSuffix) $(IntermediateDirectory)/linux.cpp$(ObjectSuffix) $(IntermediateDirectory)/Process.cpp$(ObjectSuffix) $(IntermediateDirectory)/DAPEvent.cpp$(ObjectSuffix) $(IntermediateDirectory)/UnixProcess.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/Exception.cpp$(ObjectSuffix) $(IntermediateDirectory)/ConnectionString.cpp$(ObjectSuffix) $(IntermediateDirectory)/Log.cpp$(ObjectSuffix) $(IntermediateDirectory)/ServerProtocol.cpp$(ObjectSuffix) $(IntermediateDirectory)/SocketClient.cpp$(ObjectSuffix) $(IntermediateDirectory)/JsonRPC.cpp$(ObjectSuffix) $(IntermediateDirectory)/msw.cpp$(ObjectSuffix) $(IntermediateDirectory)/SocketServer.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@if not exist "$(IntermediateDirectory)" $(MakeDirCommand) "$(IntermediateDirectory)"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@echo rebuilt > $(IntermediateDirectory)/dapcxx.relink

MakeIntermediateDirs:
	@if not exist "$(IntermediateDirectory)" $(MakeDirCommand) "$(IntermediateDirectory)"
	@if not exist "$(OutputDirectory)" $(MakeDirCommand) "$(OutputDirectory)"

$(IntermediateDirectory)/.d:
	@if not exist "$(IntermediateDirectory)" $(MakeDirCommand) "$(IntermediateDirectory)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/cJSON.cpp$(ObjectSuffix): cJSON.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/cJSON.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/cJSON.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/cJSON.cpp$(PreprocessSuffix): cJSON.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/cJSON.cpp$(PreprocessSuffix) cJSON.cpp

$(IntermediateDirectory)/dap.cpp$(ObjectSuffix): dap.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/dap.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/dap.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/dap.cpp$(PreprocessSuffix): dap.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/dap.cpp$(PreprocessSuffix) dap.cpp

$(IntermediateDirectory)/StringUtils.cpp$(ObjectSuffix): StringUtils.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/StringUtils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/StringUtils.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/StringUtils.cpp$(PreprocessSuffix): StringUtils.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/StringUtils.cpp$(PreprocessSuffix) StringUtils.cpp

$(IntermediateDirectory)/Client.cpp$(ObjectSuffix): Client.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/Client.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Client.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Client.cpp$(PreprocessSuffix): Client.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Client.cpp$(PreprocessSuffix) Client.cpp

$(IntermediateDirectory)/Socket.cpp$(ObjectSuffix): Socket.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/Socket.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Socket.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Socket.cpp$(PreprocessSuffix): Socket.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Socket.cpp$(PreprocessSuffix) Socket.cpp

$(IntermediateDirectory)/JSON.cpp$(ObjectSuffix): JSON.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/JSON.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/JSON.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/JSON.cpp$(PreprocessSuffix): JSON.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/JSON.cpp$(PreprocessSuffix) JSON.cpp

$(IntermediateDirectory)/linux.cpp$(ObjectSuffix): linux.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/linux.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/linux.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/linux.cpp$(PreprocessSuffix): linux.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/linux.cpp$(PreprocessSuffix) linux.cpp

$(IntermediateDirectory)/Process.cpp$(ObjectSuffix): Process.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/Process.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Process.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Process.cpp$(PreprocessSuffix): Process.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Process.cpp$(PreprocessSuffix) Process.cpp

$(IntermediateDirectory)/DAPEvent.cpp$(ObjectSuffix): DAPEvent.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/DAPEvent.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/DAPEvent.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/DAPEvent.cpp$(PreprocessSuffix): DAPEvent.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/DAPEvent.cpp$(PreprocessSuffix) DAPEvent.cpp

$(IntermediateDirectory)/UnixProcess.cpp$(ObjectSuffix): UnixProcess.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/UnixProcess.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/UnixProcess.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/UnixProcess.cpp$(PreprocessSuffix): UnixProcess.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/UnixProcess.cpp$(PreprocessSuffix) UnixProcess.cpp

$(IntermediateDirectory)/Exception.cpp$(ObjectSuffix): Exception.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/Exception.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Exception.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Exception.cpp$(PreprocessSuffix): Exception.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Exception.cpp$(PreprocessSuffix) Exception.cpp

$(IntermediateDirectory)/ConnectionString.cpp$(ObjectSuffix): ConnectionString.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/ConnectionString.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ConnectionString.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ConnectionString.cpp$(PreprocessSuffix): ConnectionString.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ConnectionString.cpp$(PreprocessSuffix) ConnectionString.cpp

$(IntermediateDirectory)/Log.cpp$(ObjectSuffix): Log.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/Log.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Log.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Log.cpp$(PreprocessSuffix): Log.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Log.cpp$(PreprocessSuffix) Log.cpp

$(IntermediateDirectory)/ServerProtocol.cpp$(ObjectSuffix): ServerProtocol.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/ServerProtocol.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ServerProtocol.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ServerProtocol.cpp$(PreprocessSuffix): ServerProtocol.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ServerProtocol.cpp$(PreprocessSuffix) ServerProtocol.cpp

$(IntermediateDirectory)/SocketClient.cpp$(ObjectSuffix): SocketClient.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/SocketClient.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/SocketClient.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/SocketClient.cpp$(PreprocessSuffix): SocketClient.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/SocketClient.cpp$(PreprocessSuffix) SocketClient.cpp

$(IntermediateDirectory)/JsonRPC.cpp$(ObjectSuffix): JsonRPC.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/JsonRPC.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/JsonRPC.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/JsonRPC.cpp$(PreprocessSuffix): JsonRPC.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/JsonRPC.cpp$(PreprocessSuffix) JsonRPC.cpp

$(IntermediateDirectory)/msw.cpp$(ObjectSuffix): msw.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/msw.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/msw.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/msw.cpp$(PreprocessSuffix): msw.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/msw.cpp$(PreprocessSuffix) msw.cpp

$(IntermediateDirectory)/SocketServer.cpp$(ObjectSuffix): SocketServer.cpp 
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Andrew_Development/Work_InProgress/DAP_Debugger/eranif_dgbd/dap/SocketServer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/SocketServer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/SocketServer.cpp$(PreprocessSuffix): SocketServer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/SocketServer.cpp$(PreprocessSuffix) SocketServer.cpp

##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


