# ,----------------------------------------------------------------------.
# |                                                                      |
# | sc68vlc -- Makefile.am -- Use with automake to produce configure.in  |
# |                                                                      |
# |               Copyright (C) 2010-2015 Benjamin Gerard                |
# |                                                                      |
# |               <http://sourceforge.net/users/benjihan>                |
# |                                                                      |
# `----------------------------------------------------------------------'
# @configure_input@

SUBDIRS              = $(subdirs)
ACLOCAL_AMFLAGS      = -I m4 --install
AM_CFLAGS            = $(ALL_CFLAGS) $(LIB_CFLAGS) $(VLC_CFLAGS)
AM_CPPFLAGS          = $(sc68_inc) $(file68_inc)
AM_LDFLAGS           = $(ALL_LFLAGS) $(LIB_LFLAGS) $(LFLAGS) $(VLC_LDFLAGS)

# ,----------------------------------------------------------------------.
# | Libtool plugin                                                       |
# `----------------------------------------------------------------------'

if SOURCE_SC68
SC68_LA = $(sc68_dir)/libsc68.la
@SET_MAKE@
$(sc68_dir)/libsc68.la:
	cd "$(sc68_dir)" && $(MAKE) $(AM_MAKEFLAGS) libsc68.la
else
SC68_LA = $(sc68_lib) $(file68_lib)
endif

plugindir                  = @plugindir@
plugin_LTLIBRARIES         = libsc68_plugin.la
libsc68_plugin_la_SOURCES  = sc68-vlc-demux.c sc68-vlc-vfs.c
libsc68_plugin_la_CPPFLAGS = -DMODULE_STRING='"sc68"'
libsc68_plugin_la_LIBADD   = $(SC68_LA) $(VLC_LIBS)

LIBTOOLFLAGS               = --tag=disable-static
LFLAGS                     = -shared -module -avoid-version $(NOINSTALL)
