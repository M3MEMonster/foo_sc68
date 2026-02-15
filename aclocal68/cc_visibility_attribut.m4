dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20130310 cc_visibility_attribut.m4

# AC_C_VISIBILITY_ATTRIBUT([RESULT-VAR = ac_c_visibility_attribut])
# -----------------------------------------------------------------
# Check for C compiler visibility attributs
AC_DEFUN_ONCE([AC_C_VISIBILITY_ATTRIBUT],
  [
    AC_MSG_CHECKING([for ${CC-cc} visibility attribute])
    AC_LANG_ASSERT([C])
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE(
        [
          void __attribute__ ((visibility ("hidden"))) f_hidden (void) {}
          void __attribute__ ((visibility ("internal"))) f_internal (void){}
          void __attribute__ ((visibility ("protected"))) f_protected (void){}
          void __attribute__ ((visibility ("default"))) f_default (void) {}
          int main (int argc, char **argv)
          {f_hidden();f_internal();f_protected();f_default();return 0;}
          ])],
      [ac_test_res="yes"],
      [ac_test_res="no"])
    AC_MSG_RESULT($ac_test_res)
    ifelse([$1], ,[ac_c_visibility_attribut],[$1])="$ac_test_res",
  ]
)

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of cc_visibility_attribut.m4
dnl#
dnl# ----------------------------------------------------------------------
