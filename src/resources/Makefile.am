pkgdata_DATA = \
			debugger_dap.zip

CLEANFILES = \
			$(pkgdata_DATA)

ZIP_RESOURCES = \
			breakpoints_DAP_dlg.xrc \
			debugger_DAP_options_dlg.xrc \
			debugger_DAP_project_options_dlg.xrc \
			edit_watches_DAP_dlg.xrc \
			manifest.xml

EXTRA_DIST = \
			$(ZIP_RESOURCES)
			manifest.rc

debugger_dap.zip: $(ZIP_RESOURCES)
			PWD=`pwd` cd $(srcdir) && zip $(PWD)/debugger_dap.zip $(ZIP_RESOURCES)  > /dev/null
