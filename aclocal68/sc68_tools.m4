dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20150309 sc68_tools.m4

# SC68_TOOLS()
# ------------
# Check for build tools for sc68 related package.
AC_DEFUN_ONCE([SC68_TOOLS],[
    AC_REQUIRE([SC68_SYSTEM])
    # compiler
    AC_LANG([C])
    AC_PROG_CC
    AM_PROG_CC_STDC
    AM_PROG_CC_C_O
    AC_PROG_CC_DECLSPEC
    AC_C_CONST
    AC_C_INLINE
    AC_C_VOLATILE
    AC_C_RESTRICT
    AC_C_VISIBILITY_ATTRIBUT([has_visibility])
    # tools
    AC_PROG_INSTALL
    AC_PATH_PROG([help2man],[help2man],[false help2man])
    AC_PATH_PROG([texinfo2man],[texinfo2man],[false texinfo2man])
    AC_PATH_PROG([hexdump],[hexdump],[false hexdump])
    AC_PATH_PROG([as68],[as68],[false as68]) # $$$ bootstraping problem
    AC_PATH_PROG([pkgconfig],["pkg-config"],["false"]) # or use predef macros
  ])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_tools.m4
dnl#
dnl# ----------------------------------------------------------------------
