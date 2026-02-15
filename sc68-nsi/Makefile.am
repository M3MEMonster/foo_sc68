# ,----------------------------------------------------------------------.
# |                                                                      |
# | sc68-nsi -- Makefile.am -- Use with automake to produce configure.in | 
# |                                                                      |
# |               Copyright (C) 2007-2015 Benjamin Gerard                |
# |                                                                      |
# |               <http://sourceforge.net/users/benjihan>                |
# |                                                                      |
# `----------------------------------------------------------------------'
# @configure_input@

ZIP		= @zip@
MAKENSIS	= @makensis@
installer_exe   = $(PACKAGE_TARNAME)-@sc68_version@.exe
installer_zip   = $(PACKAGE_TARNAME)-@sc68_version@.zip
uninstall_exe   = $(PACKAGE_TARNAME)-uninstall.exe

# noinst_DATA = $(installer_exe)

all: $(installer_zip)

$(installer_zip): $(installer_exe)
	$(ZIP) -0 "$@" "$<"

$(installer_exe): $(srcdir)/sc68.nsi Makefile
	"$(MAKENSIS)" $(NSISFLAGS) \
	/DNAME="@PACKAGE_NAME@" \
	/DCAPTION="SC68" \
	/DVERSION="@sc68_version@" \
	/DOUTFILE=`$(CYGPATH_W) "$${PWD}/$@"` \
	/DUNINSTALL="$(uninstall_exe)" \
	/DSRCDIR=`$(CYGPATH_W) "$(srcdir)"` \
	/DWIN32DIR=`$(CYGPATH_W) "$(win32_dir)"` \
	/DWIN64DIR=`$(CYGPATH_W) "$(win64_dir)"` \
	/DDATADIR=`$(CYGPATH_W) "$(sc68_data)"` \
	/DWMPDLL=`$(CYGPATH_W) "$(winamp_dll)"` \
	/DVLC32DLL=`$(CYGPATH_W) "$(vlc_w32_dll)"` \
	/DVLC64DLL=`$(CYGPATH_W) "$(vlc_w64_dll)"` \
	/DFOODLL=`$(CYGPATH_W) "$(fb2k_dll)"` \
	-- `$(CYGPATH_W) "$<"`

clean-local:
	-rm -f -- "$(installer_exe)" "$(uninstall_exe)" "$(installer_zip)"

EXTRA_DIST = sc68.nsi sc68-icon-32.ico
