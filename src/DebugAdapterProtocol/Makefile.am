.rc.lo:
			$(LIBTOOL) --tag=RC --mode=compile $(WX_RESCOMP) $< -o $@

AM_CPPFLAGS = \
			$(WX_CXXFLAGS) \
			$(CB_SQUIRREL_CFLAGS) \
			-I$(top_srcdir)/src/include \
			-I$(top_srcdir)/src/sdk/wxscintilla/include

debuggerwxdapDLLdir = $(libdir)

debuggerwxdapDLL_LTLIBRARIES = \
			libdebugger_wxdap.la

libdebugger_wxdap_la_LDFLAGS = \
			@MODULE_SHARED_LDFLAGS@ \
			-no-undefined \
			-avoid-version

libdebugger_wxdap_la_LIBADD = \
			$(top_srcdir)/src/sdk/libcodeblocks.la \
			$(WX_LIBS) \
			$(WX_GTK_LIBS)

libdebugger_wxdap_la_LIBADD += \
			$(CB_TINYXML_LIBS)

libdebugger_wxdap_la_SOURCES = \
			cJSON.cpp \
			Client.cpp \
			ConnectionString.cpp \
			dap.cpp \
			DAPEvent.cpp \
			Exception.cpp \
			JSON.cpp \
			JsonRPC.cpp \
			linux.cpp \
			Log.cpp \
			msw.cpp \
			Process.cpp \
			ServerProtocol.cpp \
			Socket.cpp \
			SocketClient.cpp \
			SocketServer.cpp \
			StringUtils.cpp \
			UnixProcess.cpp

noinst_HEADERS = \
			cJSON.hpp \
			Client.hpp \
			ConnectionString.hpp \
			dap.hpp \
			DAPEvent.hpp \
			dap_exports.hpp \
			Exception.hpp \
			JSON.hpp \
			JsonRPC.hpp \
			Log.hpp \
			Process.hpp \
			Queue.hpp \
			ServerProtocol.hpp \
			Socket.hpp \
			SocketClient.hpp \
			SocketServer.hpp \
			StringUtils.hpp \
			UnixProcess.hpp

EXTRA_DIST = \
			*.cbp \
			*.md \
			LICENSE