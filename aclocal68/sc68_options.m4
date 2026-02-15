dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20140310 sc68_options.m4

# SC68_OPTIONS([option-list])
# ---------------------------
# Common --enable-options for all sc68 related packages.
AC_DEFUN_ONCE([SC68_OPTIONS],[
    [#]m4_default(m4_strip([$1]),[debug] [all-static])
    SC68_ENABLE_DEBUG
    SC68_ENABLE_ALLSTATIC
    SC68_ENABLE_DUMPVARS
  ])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_options.m4
dnl#
dnl# ----------------------------------------------------------------------
