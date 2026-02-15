dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20110910 sc68_cppflags.m4

# SC68_CPPFLAGS([VAR],[CFLAGS])
# -----------------------------
# Filter CPPFLAGS out of CFLAGS into VAR
AC_DEFUN([SC68_CPPFLAGS],[
    $1=''
    set -- $2
    while test [$]# -gt 0; do
      case [$]1 in
        -D* | -I* | -U* | -undef | -nostdinc | -nostdinc++)
          $1="[$]$1 [$]1";;
      esac
      shift
    done
    $1=`echo [$]$1`
    ])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_cppflags.m4
dnl#
dnl# ----------------------------------------------------------------------
