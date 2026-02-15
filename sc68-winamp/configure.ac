dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sc68-winamp
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [sc68-winamp],
  [0.5],
  [http://sourceforge.net/projects/sc68/],
  [sc68-winamp],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([in_sc68.c])
AC_CONFIG_HEADERS([config.h])


# ,----------------------------------------------------------------------.
# | build tools                                                          |
# `----------------------------------------------------------------------'

AC_CHECK_TOOL([WINDRES],[windres],[false])
dnl # if test x/"$WINDRES" = x/false; then
dnl #   AC_MSG_ERROR([Missing required build tool -- windres])
dnl # fi

AC_CHECK_TOOL([STRIP],[strip],[false])
if test x/"$STRIP" = x/false; then
  AC_MSG_ERROR([Missing required build tool -- strip])
fi


# ,----------------------------------------------------------------------.
# | Locate sdks                                                          |
# `----------------------------------------------------------------------'

AC_ARG_WITH(
  [winamp-sdk],
  [AS_HELP_STRING([--with-winamp-sdk],[locate winamp SDK])])

# AC_ARG_WITH(
#   [windows-sdk],
#   [AS_HELP_STRING([--with-windows-sdk],[locate windows platform SDK])])

AC_ARG_WITH(
  [winamp-dir],
  [AS_HELP_STRING([--with-winamp-dir],[locate winamp install])])

# ,----------------------------------------------------------------------.
# | Winamp SDK                                                           |
# `----------------------------------------------------------------------'

if test s[$]{wa_sdk+et} != set; then
  if test s[$]{with_winamp_sdk+et} = set; then
    wa_sdk="[$]{with_winamp_sdk}"
  elif test -d ./winamp-sdk/; then
    wa_sdk=./winamp-sdk
  fi
fi

save_CPPFLAGS="[$]CPPFLAGS"
if test s[$]{wa_sdk+et} = set; then
  CPPFLAGS="[$]CPPFLAGS -I[$]{wa_sdk}"
fi
AC_CHECK_HEADERS([winamp/in2.h],,AC_MSG_ERROR([See above]))
CPPFLAGS="[$]save_CPPFLAGS"

# ,----------------------------------------------------------------------.
# | Winamp Install Dir                                                   |
# `----------------------------------------------------------------------'
if test s[$]{wa_dir+et} != set; then
  if test s[$]{with_winamp_dir+et} = set; then
    wa_dir="[$]{with_winamp_dir}"
  elif test -d ./winamp-dir/; then
    wa_dir=./winamp-dir
  else
    reg="`which reg 2>/dev/null`"
    if test x"[$]reg" != x; then
      windir=`"[$]reg" QUERY 'HKEY_CURRENT_USER\Software\Winamp' | sed -n 's|^.* \+REG_SZ \+\(.*\)[$]|\1|p'`
      if test x"[$]windir" != x; then
        if which cygpath >/dev/null 2>/dev/null; then
          wa_dir=`cygpath "$windir"`
        else
          wa_dir="[$]windir"
        fi
      fi

    fi
  fi
fi

# ,----------------------------------------------------------------------.
# | Windows Platform SDK                                                 |
# `----------------------------------------------------------------------'

# if test s[$]{wp_sdk+et} != set; then
#   if test s[$]{with_windows_sdk+et} = set; then
#     wp_sdk="[$]{with_windows_sdk}"
#   elif test -d ./windows-sdk/; then
#       wp_sdk=./windows-sdk
#   fi
# fi

# save_CPPFLAGS="[$]CPPFLAGS"
# if test s[$]{wp_sdk+et} = set; then
#   CPPFLAGS="[$]CPPFLAGS -I[$]{wp_sdk}"
# fi
# AC_CHECK_HEADERS([windows.h],,AC_MSG_ERROR([See above]))
# CPPFLAGS="[$]save_CPPFLAGS"

 
export [PKG_CONFIG_PATH]="[$]prefix/lib/pkgconfig"
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([sc68],[sc68 >= 3.0.0])
AC_MSG_CHECKING([sc68 version])
sc68_VERSION=`[$]PKG_CONFIG [sc68] --modversion`
AC_MSG_RESULT([[$]sc68_VERSION])

save_CPPFLAGS="[$]CPPFLAGS"
CPPFLAGS="[$]sc68_CFLAGS [$]CPPFLAGS"
AC_CHECK_HEADERS([sc68/sc68.h],,AC_MSG_ERROR([See above]))
CPPFLAGS="[$]save_CPPFLAGS"

save_LIBS="[$]LIBS"
LIBS="[$]sc68_LIBS [$]LIBS"
AC_CHECK_FUNCS([sc68_version],,AC_MSG_ERROR([See above]))
LIBS="[$]save_LIBS"

# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

AC_SUBST([wa_sdk])
# AC_SUBST([wp_sdk])
AC_SUBST([wa_dir])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# ,----------------------------------------------------------------------.
# | Summary                                                              |
# `----------------------------------------------------------------------'

AC_MSG_NOTICE([])
AC_MSG_NOTICE([,-----------------------])
AC_MSG_NOTICE([| $PACKAGE_NAME ($PACKAGE_VERSION)])
AC_MSG_NOTICE([+-----------------------])
AC_MSG_NOTICE([|       sc68 : [$]sc68_VERSION])
AC_MSG_NOTICE([| winamp sdk : [$]wa_sdk])
AC_MSG_NOTICE([|  win32 sdk : [$]wp_sdk])
AC_MSG_NOTICE([| winamp dir : [$]wa_dir])
AC_MSG_NOTICE([+-----------------------])
