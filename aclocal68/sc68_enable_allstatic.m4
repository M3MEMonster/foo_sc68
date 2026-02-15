dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20140311 sc68_enable_allstatic.m4

# SC68_ENABLE_ALLSTATIC([MORE-ENABLE-ACTION],[MORE-DISABLE-ACTION])
# -----------------------------------------------------------------
# Defines --enable-sc68-static option
AC_DEFUN_ONCE([SC68_ENABLE_ALLSTATIC],[
    AC_ARG_ENABLE(
      [sc68-static],
      [AS_HELP_STRING([--enable-sc68-static],
	  [all static linkage (yes,no,lib) @<:@default=no@:>@])],
      [],[enable_sc68_static=no])
    ])

# SC68_DO_ALLSTATIC
# -----------------
# Apply the standalone dll trickery
AC_DEFUN_ONCE([SC68_DO_ALLSTATIC],[
    if test X"[$]enable_sc68_static" = Xlib; then
      sc68_wl="-Wl,-dn"
      set -- [$]ALL_LFLAGS [$]LIBS
      ALL_LFLAGS=''
      while test [$]# -gt 0; do
        case X[$]1 in
          X-l*)
            sc68_wl="[$]sc68_wl,[$]1"
            ;;
          *)
            ALL_LFLAGS="[$]ALL_LFLAGS [$]1"
            ;;
        esac
        shift
      done
      if test x"[$]sc68_wl" = x'-Wl,-dn'; then
        sc68_wl=''
      else
        sc68_wl="[$]sc68_wl,-dy"
      fi
      ALL_LFLAGS="[$](echo [$]ALL_LFLAGS)"
      LIBS="[$]sc68_wl"
    fi
    ])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_enable_allstatic.m4
dnl#
dnl# ----------------------------------------------------------------------
