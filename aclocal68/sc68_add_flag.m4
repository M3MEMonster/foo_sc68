dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20140310 sc68_add_flag.m4

# SC68_ADD_FLAG([VAR],[FLAG])
# ---------------------------
# Add unique FLAG to VAR
AC_DEFUN([SC68_ADD_FLAG],[
    if test X"[$]$1" = X ; then
      $1="$2"
    else
      for sc68_add_flag in [$]$1 ; do
        test X"[$]sc68_add_flag" = X"$2" && break
      done;
      if test [$]? -ne 0; then
        $1="[$]$1 $2"
      fi
      unset sc68_add_flag
    fi
  ])

# SC68_ADD_FLAGS([VAR],[FLAGS])
# -----------------------------
# Add unique FLAGS to VAR
AC_DEFUN([SC68_ADD_FLAGS],[
    set -- $2
    while test [$]# -gt 0; do
      SC68_ADD_FLAG($1,[$]1)
      shift
    done
  ])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_add_flag.m4
dnl#
dnl# ----------------------------------------------------------------------
