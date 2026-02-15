dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sc68-nsis
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [sc68-nsis],
  [1.0],
  [http://sourceforge.net/projects/sc68/],
  [sc68-nsis],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([sc68.nsi])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
		  no-installman no-installinfo])

# ,----------------------------------------------------------------------.
# | Locate pre-installed sc68 files (win32 / win64 / data)               |
# `----------------------------------------------------------------------'

AC_ARG_WITH(
  [win32-path],
  [AS_HELP_STRING([--with-win32-path],[locate sc68 win32 binaries])])

AC_ARG_WITH(
  [win64-path],
  [AS_HELP_STRING([--with-win64-path],[locate sc68 win64 binaries])])

AC_ARG_WITH(
  [data-path],
  [AS_HELP_STRING([--with-data-path],
      [locate sc68 architecture independent data])])

AC_ARG_WITH(
  [winamp-dll],
  [AS_HELP_STRING([--with-winamp-dll],[locate sc68 winamp plugin])])

AC_ARG_WITH(
  [vlc-w32-dll],
  [AS_HELP_STRING([--with-vlc-w32-dll],[locate sc68 vlc win32 plugin])])

AC_ARG_WITH(
  [vlc-w64-dll],
  [AS_HELP_STRING([--with-vlc-w64-dll],[locate sc68 vlc win64 plugin])])

AC_ARG_WITH(
  [fb2k-dll],
  [AS_HELP_STRING([--with-fb2k-dll],[locate sc68 foobar2000 plugin])])

AC_PATH_PROG([makensis],[makensis])
AC_PATH_PROG([zip],[zip])

for winnn in win32 win64; do
  AC_MSG_CHECKING([sc68 [$]winnn binary path])
  if ! eval test -d "\"[\$]with_[$]{winnn}_path\""; then
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([see above]) 
  fi
  AC_MSG_RESULT([yes])
  eval [$]{winnn}_dir="\"[\$]with_[$]{winnn}_path\""

  for exe in sc68 unice68 info68 mksc68; do
    AC_MSG_CHECKING([sc68 [$]winnn [$]{exe}.exe])
    if eval test -r "\"[\$][$]{winnn}_dir/bin/[$]{exe}.exe\""; then
      eval [$]{winnn}_[$]{exe}="\"[\$][$]{winnn}_dir/bin/[$]{exe}.exe\""
      AC_MSG_RESULT([yes])
    else
      eval win32_[$]{exe}=''
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([see above]) 
    fi
  done
done

for plg in winamp fb2k vlc_w32 vlc_w64; do
  AC_MSG_CHECKING([sc68 [$]plg plugin dll])
  if eval test -r "\"[\$]with_[$]{plg}_dll\""; then
      eval [$]{plg}_dll="\"[\$]with_[$]{plg}_dll\""
      AC_MSG_RESULT([yes])
    else
      eval [$]{plg}_dll=''
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([see above]) 
    fi
done

AC_MSG_CHECKING([sc68 architecture independant data])
if test -d "[$]{with_data_path}/Replay"; then
  sc68_data="[$]{with_data_path}"
  AC_MSG_RESULT([yes])
else
  sc68_data=''
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([see above]) 
fi

export [PKG_CONFIG_PATH]="[$]win32_dir/lib/pkgconfig"
PKG_PROG_PKG_CONFIG
AC_MSG_CHECKING([sc68 version])
sc68_version=`[$]PKG_CONFIG [sc68] --modversion`
AC_MSG_RESULT([[$]sc68_version])


# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

AC_SUBST([win32_dir])
AC_SUBST([win32_sc68])
AC_SUBST([win32_unice68])
AC_SUBST([win32_info68])
AC_SUBST([win32_mksc68])
AC_SUBST([win64_dir])
AC_SUBST([win64_sc68])
AC_SUBST([win64_unice68])
AC_SUBST([win64_info68])
AC_SUBST([win64_mksc68])
AC_SUBST([sc68_data])
AC_SUBST([sc68_version])
AC_SUBST([vlc_w32_dll])
AC_SUBST([vlc_w64_dll])
AC_SUBST([winamp_dll])
AC_SUBST([fb2k_dll])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# ,----------------------------------------------------------------------.
# | Summary                                                              |
# `----------------------------------------------------------------------'

AC_MSG_NOTICE([])
AC_MSG_NOTICE([,-----------------------])
AC_MSG_NOTICE([| $PACKAGE_NAME ($PACKAGE_VERSION)])
AC_MSG_NOTICE([+-----------------------])
AC_MSG_NOTICE([|       sc68 : [$]sc68_version])
AC_MSG_NOTICE([| win32 path : [$]win32_dir])
AC_MSG_NOTICE([| win64 path : [$]win64_dir])
AC_MSG_NOTICE([|  data path : [$]sc68_data])
AC_MSG_NOTICE([| winamp dll : [$]winamp_dll])
AC_MSG_NOTICE([| vlc-32 dll : [$]vlc_w32_dll])
AC_MSG_NOTICE([| vlc-64 dll : [$]vlc_w64_dll])
AC_MSG_NOTICE([|   fb2k dll : [$]fb2k_dll])
AC_MSG_NOTICE([|   makensis : [$]makensis])
AC_MSG_NOTICE([+-----------------------])
