dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20140310 sc68_package.m4

# SC68_PACKAGE([DESC])
# --------------------
# Common package info for sc68 related packages.
AC_DEFUN_ONCE([SC68_PACKAGE],[
    AC_PACKAGE_INFO(
      [$1 It is part of the sc68 project.])
    [PACKAGE_INFOCAT='sc68 - /|\ Atari ST and C= Amiga music player']
    AC_SUBST([PACKAGE_INFOCAT])
    m4_define([PKG],AS_TR_CPP(AC_PACKAGE_NAME))
    ALL_CFLAGS=''; BIN_CFLAGS=''; LIB_CFLAGS=''; PAC_CFLAGS=''
    ALL_LFLAGS=''; BIN_LFLAGS=''; LIB_LFLAGS=''; PAC_LFLAGS=''
    PAC_REQUIRES=''; PAC_CONFLICTS=''; PAC_PRIVLIB=''; PAC_PRIVREQ=''
    ])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_package.m4
dnl#
dnl# ----------------------------------------------------------------------
