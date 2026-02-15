dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20140310 cc_declspec.m4

# AC_PROG_CC_DECLSPEC()
# ------------------------------
AC_DEFUN_ONCE([AC_PROG_CC_DECLSPEC],
  [
    AC_MSG_CHECKING([for cc __declspec()])
    AC_LANG_ASSERT([C])
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE(
        [
          __declspec(dllexport) void f_exported (void);
          void f_exported (void) {}
          __declspec(dllimport) void f_imported (void);
          int main (int argc, char **argv)
          {f_exported();f_imported();return 0;}
          ])],
      [ac_cc_declspec="yes"],
      [ac_cc_declspec="no"]
    )
    AC_MSG_RESULT($ac_cc_declspec)
  ]
)

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of cc_declspec.m4
dnl#
dnl# ----------------------------------------------------------------------
