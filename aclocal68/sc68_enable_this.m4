dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20140310 sc68_enable_this.m4

# SC68_ENABLE_THIS([SH-VAR])
# --------------------------
# Test if package in list and set it accordingly
AC_DEFUN([SC68_ENABLE_THIS],[
    AC_MSG_CHECKING([for m4_translit([$1],[_],[ ])])
    case X"[$]$1" in
      Xyes | Xno) ;;
      *)
        set -- [$](echo [$]$1|sed 's/,/ /g')
        $1=no
        while test [$]# -gt 0; do
          if test X'AC_PACKAGE_NAME' = X"[$]1"; then
            $1=yes; break
          fi
          shift
        done
    esac
    AC_MSG_RESULT([$]$1)
    AS_TR_SH(AC_PACKAGE_NAME)_$1=[$]$1
  ])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_enable_this.m4
dnl#
dnl# ----------------------------------------------------------------------
