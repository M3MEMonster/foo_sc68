dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for cdbg68
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [cdbg68],
  [20090618],
  [http://sourceforge.net/projects/sc68/],
  [cdbg68],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([src/cdbg68.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
                  no-installman no-installinfo])
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([win32-dll])

# ,----------------------------------------------------------------------.
# | sc68                                                                 |
# `----------------------------------------------------------------------'

SC68_PACKAGE([
    68K curses debugger.
    Terminal mode debugger for sc68 files.
  ])
SC68_SYSTEM
SC68_OPTIONS
SC68_TOOLS
SC68_CHECKS
SC68_WITH([desa68],[yes])
SC68_WITH([sc68],[yes],[sc68_version],[sc68/sc68.h],[libsc68])
SC68_WITH_HEADER([file68],[yes],[sc68/file68.h])

# ,----------------------------------------------------------------------.
# | extra libraries                                                      |
# `----------------------------------------------------------------------'
SC68_WITH([ncurses],[yes],[curses_version],[curses.h])

# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
#     package.texi
#     sourcer68-${VERSION}.ebuild:sourcer68.ebuild.in
#     sourcer68.1.tpl
#     sourcer68.spec])
AC_OUTPUT

# ,----------------------------------------------------------------------.
# | Summary                                                              |
# `----------------------------------------------------------------------'

SC68_DUMPVARS

AC_MSG_NOTICE([])
AC_MSG_NOTICE([,-----------------------])
AC_MSG_NOTICE([| $PACKAGE_NAME ($PACKAGE_VERSION)])
AC_MSG_NOTICE([|-----------------------])
AC_MSG_NOTICE([|   compile mode        : $compile_mode])
AC_MSG_NOTICE([|   desa68              : $has_desa68 ($desa68_ver)])
AC_MSG_NOTICE([|   sc68                : $has_sc68 ($sc68_ver)])
AC_MSG_NOTICE([+-----------------------])
