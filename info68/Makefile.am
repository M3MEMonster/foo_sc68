# ,----------------------------------------------------------------------.
# |                                                                      |
# |  info68 -- Makefile.am -- Use with automake to produce Makefile.in   | 
# |                                                                      |
# |               Copyright (C) 2006-2015 Benjamin Gerard                |
# |                                                                      |
# |               <http://sourceforge.net/users/benjihan>                |
# |                                                                      |
# `----------------------------------------------------------------------'
# @configure_input@

SUBDIRS              = @subdirs@
ACLOCAL_AMFLAGS      = -I m4 --install

AM_CFLAGS            = $(ALL_CFLAGS) $(BIN_CFLAGS) $(file68_inc)
AM_LDFLAGS           = $(ALL_LFLAGS) $(BIN_LFLAGS)

# ,----------------------------------------------------------------------.
# | Libtool program                                                      |
# `----------------------------------------------------------------------'

if SOURCE_FILE68
FILE68_LA = $(file68_dir)/libfile68.la
@SET_MAKE@
$(file68_dir)/libfile68.la:
	cd "$(file68_dir)" && $(MAKE) $(AM_MAKEFLAGS) libfile68.la
else
FILE68_LA            =
endif

bin_PROGRAMS         = info68
info68_SOURCES       = info68.c
info68_LDADD         = $(FILE68_LA) $(DESA68_LA)
noinst_HEADERS       =

# ,----------------------------------------------------------------------.
# | Documentation                                                        |
# `----------------------------------------------------------------------'

# dist_man1_MANS        = info68.man

# info_TEXINFOS        = info68.texi
# MAINTAINERCLEANFILES = $(srcdir)/info68.1

# TEXINFO2MAN          = $(texinfo2man)	
# fail                 = { test -e '$@' && rm '$@'; test a = b; }

# $(srcdir)/info68.1: info68.1.tpl.in $(srcdir)/info68.texi
# 	$(TEXINFO2MAN) info68.1.tpl $(srcdir)/info68.texi >$@ || $(fail)
