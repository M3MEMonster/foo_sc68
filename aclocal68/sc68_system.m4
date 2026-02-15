dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20130723 sc68_system.m4

# SC68_SYSTEM()
# -------------
# Setup host and build for all sc68 related packages.
AC_DEFUN_ONCE([SC68_SYSTEM],[
    AC_REQUIRE([AC_CANONICAL_BUILD])
    AC_REQUIRE([AC_CANONICAL_HOST])

    # other system related tests
    AC_USE_SYSTEM_EXTENSIONS
    AC_SYS_LARGEFILE
    AC_SYS_LONG_FILE_NAMES

    # build for win32 platform ?
    AC_MSG_CHECKING([for some win32 platform])
    ac_sys_platform_win32=no
    case "[$]host" in
      *-mingw*|*-cygwin*|*-win*)
        ac_sys_platform_win32=yes ;;
    esac
    AC_MSG_RESULT([$ac_sys_platform_win32])

    # build for native win32 platform (w/o un*x style layout)
    AC_MSG_CHECKING([for native win32 platform])
    ac_sys_native_win32=no
    case x-"[$]host_os" in
      x-mingw32 | x-win32)
        AC_DEFINE([NATIVE_WIN32],[1],[Native win32 subsystem])
        ac_sys_native_win32=yes ;;
    esac
    AC_MSG_RESULT([$ac_sys_native_win32])
  ]
)

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_system.m4
dnl#
dnl# ----------------------------------------------------------------------
