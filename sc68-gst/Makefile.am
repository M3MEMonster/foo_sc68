# ,----------------------------------------------------------------------.
# |                                                                      |
# | gstsc68 -- Makefile.am -- Use with automake to produce configure.in  |
# |                                                                      |
# |               Copyright (C) 2010-2015 Benjamin Gerard                |
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

if TYPEFIND_SC68
more_ltlib                 = libgstsc68-tf.la
libgstsc68_tf_la_SOURCES   = gstsc68_typefind.c
use_typefinder             = 1
else
more_ltlib                 =
use_typefinder             = 0
endif

plugin_LTLIBRARIES         = libgstsc68.la $(more_ltlib)
libgstsc68_la_SOURCES      = gstsc68.c gstsc68_core.c
libgstsc68_la_CPPFLAGS     = -DUSE_TYPEFINDER=$(use_typefinder) -DPLUGINDIR=\"$(plugindir)\"
libgstsc68_la_LIBADD       = $(SC68_LA)
noinst_HEADERS             = gstsc68.h

LIBTOOLFLAGS               = --tag=disable-static
LFLAGS                     = -shared -module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).* $(NOINSTALL)
