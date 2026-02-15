dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sc68-vlc
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [sc68-vlc],
  [0.8.0],
  [http://sourceforge.net/projects/sc68/],
  [sc68-vlc],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([sc68-vlc-demux.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
                  no-installman no-installinfo])
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([win32-dll])

# ,----------------------------------------------------------------------.
# | sc68                                                                 |
# `----------------------------------------------------------------------'

SC68_PACKAGE([Atari ST and Amiga audio demuxer for vlc.])
SC68_SYSTEM
SC68_OPTIONS
SC68_TOOLS
SC68_CHECKS
SC68_WITH([sc68],[yes],[sc68_version],[sc68/sc68.h],[libsc68])
SC68_WITH_HEADER([file68],[yes],[sc68/file68.h])

# ,----------------------------------------------------------------------.
# | vlc                                                                  |
# `----------------------------------------------------------------------'

vlc_ver='n/a'         # get that later from pkg-config
vlc_min='2.1.0'       # Not sure exactly which versions are supported

PKG_CHECK_MODULES([VLC],[vlc-plugin >= [$]vlc_min])

vlc_ver=[$]([$]pkgconfig --modversion vlc-plugin || echo 'n/a')
if test x[$]{VLC_PLUGIN_DIR+set} = xset; then
    vlc_dir="[$]{VLC_PLUGIN_DIR}"
else
  vlc_dir=[$]([$]pkgconfig --variable=pluginsdir vlc-plugin)
fi

if test x"[$]{vlc_dir}" = x; then
    AC_MSG_ERROR([Unable to locate vlc plugin dir (set VLC_PLUGIN_DIR)])
fi

plugindir=[$]{vlc_dir}/demux
AC_SUBST([plugindir])

# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

AC_CONFIG_FILES([
    Makefile])
AC_OUTPUT

# ,----------------------------------------------------------------------.
# | Summary                                                              |
# `----------------------------------------------------------------------'

SC68_DUMPVARS

AC_MSG_NOTICE([])
AC_MSG_NOTICE([,-----------------------])
AC_MSG_NOTICE([| $PACKAGE_NAME ($PACKAGE_VERSION)])
AC_MSG_NOTICE([|-----------------------])
AC_MSG_NOTICE([|   file68 headers      : $has_file68 ($file68_ver)])
AC_MSG_NOTICE([|   libsc68             : $has_sc68 ($sc68_ver)])
AC_MSG_NOTICE([|   vlc                 : yes ($vlc_ver)])
AC_MSG_NOTICE([|   plugin-dir          : $vlc_dir])
AC_MSG_NOTICE([+-----------------------])
