dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl# 
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sc68-ng
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [sc68-ng],
  [3.0.0b],
  [http://sourceforge.net/projects/sc68/],
  [sc68-ng],
  [http://sc68.atari.org])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
                  no-installman no-installinfo])
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([win32-dll])

# ,----------------------------------------------------------------------.
# | sc68                                                                 |
# `----------------------------------------------------------------------'

SC68_PACKAGE([Atari ST and Amiga music player.])
SC68_SYSTEM
SC68_OPTIONS
SC68_TOOLS
SC68_CHECKS

# ,----------------------------------------------------------------------.
# | Optional Packages                                                    |
# `----------------------------------------------------------------------'

AC_ARG_ENABLE(
  [sc68-tools],
  [AS_HELP_STRING(
      [--enable-sc68-tools],
      [optionnal developer tools @<:@default=no@:>@])],
  [],[enable_sc68_tools=no])

AC_ARG_ENABLE(
  [sc68-plugin],
  [AS_HELP_STRING(
      [--enable-sc68-plugin],
      [enable plugin-packages @<:@default=no@:>@])],
  [],[enable_sc68_plugin="check"])

AC_ARG_ENABLE(
  [sc68-all],
  [AS_HELP_STRING(
      [--enable-sc68-all],
      [enable all sub-packages @<:@default=no@:>@])],
  [],[enable_sc68_all="check"])

AS_CASE(["x$enable_sc68_all"],
  [xno|xyes],
  [
    enable_sc68_tools="[$]enable_sc68_all"
    enable_sc68_plugin="[$]enable_sc68_all"
    ],
  [enable_sc68_all="check"])
enable_sc68_cli="[$]enable_sc68_all"
enable_sc68_lib="[$]enable_sc68_all"

# Dev Tools
SC68_ENABLE_PACKAGE([as68],[68k macro assembler],[[$]enable_sc68_tools])
SC68_ENABLE_PACKAGE([sourcer68],[68k sourcer],[[$]enable_sc68_tools])
SC68_ENABLE_PACKAGE([mksc68],[sc68 toolbox],[[$]enable_sc68_tools])
SC68_ENABLE_PACKAGE([desa68],[68k disassembler library],[[$]enable_sc68_tools])

# API Libraries
SC68_ENABLE_PACKAGE([unice68],[ice depacker support],[[$]enable_sc68_lib])
SC68_ENABLE_PACKAGE([file68],[file68 library],[[$]enable_sc68_lib])
SC68_ENABLE_PACKAGE([libsc68],[sc68 engine],[[$]enable_sc68_lib])

# CLI programs
SC68_ENABLE_PACKAGE([info68],[sc68 files info program],[[$]enable_sc68_cli])
SC68_ENABLE_PACKAGE([sc68],[sc68 command line player],[[$]enable_sc68_cli])

# PLUGINS
def_audacious=no; def_sc68_gst=no; def_sc68_vlc=no
AS_CASE(["x$enable_sc68_plugin"],
  [xno|xyes],
  [
    def_audacious=[$]enable_sc68_plugin
    def_sc68_gst=[$]enable_sc68_plugin
    def_sc68_vlc=[$]enable_sc68_plugin],
  [
    $pkgconfig audacious --exists && def_audacious=check
    $pkgconfig gstreamer-0.10 --exists && def_sc68_gst=check
    $pkgconfig vlc-plugin --exists && def_sc68_vlc=check
  ])
SC68_ENABLE_PACKAGE([sc68-audacious],[sc68 plugin for audacious],[$def_audacious])
SC68_ENABLE_PACKAGE([sc68-gst],[sc68 plugin for gstreamer (0.10)],[$def_sc68_gst])
SC68_ENABLE_PACKAGE([sc68-vlc],[sc68 plugin for vlc],[$def_sc68_vlc])

# DOC
SC68_ENABLE_PACKAGE([sc68-doc],[sc68 documentation],[no])

# ,----------------------------------------------------------------------.
# | Package Dependencies                                                 |
# `----------------------------------------------------------------------'

## Don't check dependencies on the source tree. Some packages may not
## be present here but available by others means (may be installed or
## out of tree sources).

# ,----------------------------------------------------------------------.
# | Subdirs                                                              |
# `----------------------------------------------------------------------'

if test Xyes = X"${enable_as68}"
then AC_CONFIG_SUBDIRS([as68])
fi

if test Xyes = X"${enable_desa68}"
then AC_CONFIG_SUBDIRS([desa68])
fi

if test Xyes = X"${enable_unice68}"
then AC_CONFIG_SUBDIRS([unice68])
fi

if test Xyes = X"${enable_file68}"
then AC_CONFIG_SUBDIRS([file68])
fi

if test Xyes = X"${enable_info68}"
then AC_CONFIG_SUBDIRS([info68])
fi

if test Xyes = X"${enable_sourcer68}"
then AC_CONFIG_SUBDIRS([sourcer68])
fi

if test Xyes = X"${enable_libsc68}"
then AC_CONFIG_SUBDIRS([libsc68])
fi

if test Xyes = X"${enable_sc68}"
then AC_CONFIG_SUBDIRS([sc68])
fi

if test Xyes = X"${enable_sc68_audacious}"
then AC_CONFIG_SUBDIRS([sc68-audacious])
fi

if test Xyes = X"${enable_sc68_gst}"
then AC_CONFIG_SUBDIRS([sc68-gst])
fi

if test Xyes = X"${enable_sc68_vlc}"
then AC_CONFIG_SUBDIRS([sc68-vlc])
fi

if test Xyes = X"${enable_mksc68}"
then AC_CONFIG_SUBDIRS([mksc68])
fi

if test Xyes = X"${enable_sc68_doc}"
then AC_CONFIG_SUBDIRS([sc68-doc])
fi

AC_SUBST([subdirs])

# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# ,----------------------------------------------------------------------.
# | Summary                                                              |
# `----------------------------------------------------------------------'

[subdirs=$(echo $subdirs)]
AC_MSG_NOTICE([])
AC_MSG_NOTICE([,-----------------------])
AC_MSG_NOTICE([| $PACKAGE_NAME ($PACKAGE_VERSION)])
AC_MSG_NOTICE([|-----------------------])
AC_MSG_NOTICE([|   compile mode        : $compile_mode])
AC_MSG_NOTICE([|   sub-dirs            : $subdirs])
AC_MSG_NOTICE([+-----------------------])
AC_MSG_NOTICE([])
