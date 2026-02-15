# ,----------------------------------------------------------------------.
# |                                                                      |
# | sourcer68 -- Makefile.am -- Use with automake to produce configure.in|
# |                                                                      |
# |               Copyright (C) 2006-2015 Benjamin Gerard                |
# |                                                                      |
# |               <http://sourceforge.net/users/benjihan>                |
# |                                                                      |
# `----------------------------------------------------------------------'
# @configure_input@

SUBDIRS              = $(subdirs)
ACLOCAL_AMFLAGS      = -I m4 --install

AM_CFLAGS            = $(ALL_CFLAGS) $(BIN_CFLAGS)
AM_CPPFLAGS          = $(desa68_inc) $(file68_inc)
AM_LDFLAGS           = $(ALL_LFLAGS) $(BIN_LFLAGS)

# ,----------------------------------------------------------------------.
# | Libtool program                                                      |
# `----------------------------------------------------------------------'

if SOURCE_DESA68
DESA68_LA = $(desa68_dir)/libdesa68.la
@SET_MAKE@
$(desa68_dir)/libdesa68.la:
	$(MAKE) -C "$(desa68_dir)" $(AM_MAKEFLAGS) libdesa68.la
else
DESA68_LA = 
endif

if SOURCE_FILE68
FILE68_LA = $(file68_dir)/libfile68.la
@SET_MAKE@
$(file68_dir)/libfile68.la:
	$(MAKE) -C "$(file68_dir)" libfile68.la
else
FILE68_LA =
endif

bin_PROGRAMS      = sourcer68
sourcer68_SOURCES = sourcer68.c src68_def.h src68_exe.c src68_exe.h	\
 src68_eva.c src68_eva.h src68_sec.c src68_sec.h src68_mbk.c		\
 src68_mbk.h src68_tos.c src68_tos.h src68_sym.c src68_sym.h		\
 src68_opt.c src68_opt.h src68_msg.c src68_msg.h src68_vec.c		\
 src68_vec.h src68_adr.c src68_adr.h src68_dis.c src68_dis.h		\
 src68_src.c src68_src.h src68_fmt.c src68_fmt.h
sourcer68_LDADD   = $(FILE68_LA) $(DESA68_LA)

# ,----------------------------------------------------------------------.
# | Documentation                                                        |
# `----------------------------------------------------------------------'

# dist_man1_MANS       = sourcer68.man
# info_TEXINFOS        = sourcer68.texi
# MAINTAINERCLEANFILES = $(srcdir)/sourcer68.1

# TEXINFO2MAN          = $(texinfo2man)
# fail                 = { test -e '$@' && rm '$@'; test a = b; }

# $(srcdir)/sourcer68.1: sourcer68.1.tpl.in $(srcdir)/sourcer68.texi
# 	$(TEXINFO2MAN) sourcer68.1.tpl $(srcdir)/sourcer68.texi >$@ || $(fail)
