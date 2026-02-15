# ,----------------------------------------------------------------------.
# |                                                                      |
# |  sc68aud -- Makefile.am -- Use with automake to produce Makefile.in  | 
# |                                                                      |
# |               Copyright (C) 2007-2015 Benjamin Gerard                |
# |                                                                      |
# |               <http://sourceforge.net/users/benjihan>                |
# |                                                                      |
# `----------------------------------------------------------------------'
# @configure_input@

SUBDIRS              = $(subdirs)
ACLOCAL_AMFLAGS      = -I m4 --install
AM_CFLAGS            = $(ALL_CFLAGS) $(LIB_CFLAGS) $(BIN_CFLAGS)
AM_CPPFLAGS          = $(sc68_inc) $(file68_inc) 
AM_LDFLAGS           = $(ALL_LFLAGS) $(LIB_LFLAGS) $(BIN_LFLAGS) $(LFLAGS)

# ,----------------------------------------------------------------------.
# | Libtool program                                                      |
# `----------------------------------------------------------------------'

if SOURCE_SC68
SC68_LA = $(sc68_dir)/libsc68.la
@SET_MAKE@
$(sc68_dir)/libsc68.la:
	cd "$(sc68_dir)" && $(MAKE) $(AM_MAKEFLAGS) libsc68.la
else
SC68_LA = $(sc68_lib) $(file68_lib)
endif

plugin_LTLIBRARIES = sc68.la
if AUDACIOUS_AT_LEAST_35
cfiles = sc68-aud35.c
else
cfiles = sc68-audacious.c
endif
sc68_la_SOURCES    = $(cfiles)
sc68_la_LIBADD     = $(SC68_LA)
LFLAGS             = -module -avoid-version $(NOINSTALL)
