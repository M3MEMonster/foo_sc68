dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20140310 sc68_enable_package.m4

# TODO: missing Dirname in parameter list

# $1=Package $2=Varname $3=Description $4=Default-Value
m4_define([SC68__ENABLE_PACKAGE],[
    AC_ARG_ENABLE(
      [$1],
      [AS_HELP_STRING([--enable-$1],[$3 @<:@default=$4@:>@])],
      [],[enable_]$2[="$4"])
    case "[$][enable_]$2" in
      no | yes | check) ;;
      *)
        AC_MSG_ERROR(
          [invalid value ([$]enable_$2) for --enable-$1 (use no/yes/check)])
        ;;
    esac
    case [$]enable_$2 in
      no) has_$2=no ;;
      *)
        AC_MSG_CHECKING([whether sub-package $1 exists])
        $2_dir='[$]srcdir/$1'
        if test -d "[$]srcdir/$1"; then
          has_$2=yes
        else
          has_$2=no
        fi
        AC_MSG_RESULT([[$]has_$2 ([$]$2_dir)])
        if test "[$]{has_$2}x[$]{enable_$2}" = "noxyes"; then
          AC_MSG_ERROR([unable to configure requested package ($1)])
        fi
        ;;
    esac
    enable_$2="[$]has_$2"
  ])

# SC68_ENABLE_PACKAGE([PACKAGE],[DESC],[DEFVAL])
# ----------------------------------------------
# Defines --enable-<sub-package> configure option
AC_DEFUN([SC68_ENABLE_PACKAGE],[SC68__ENABLE_PACKAGE(
      [$1],m4_translit([$1],[-],[_]),
      m4_default([$2],[optionnal package $1]),
      m4_default([$3],[check]))])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_enable_package.m4
dnl#
dnl# ----------------------------------------------------------------------
