dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sourcer68
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [sourcer68],
  [20150319],
  [http://sourceforge.net/projects/sc68/],
  [sourcer68],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([sourcer68.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
                  no-installman no-installinfo])
AC_CONFIG_HEADERS([config.h])
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([win32-dll])

# ,----------------------------------------------------------------------.
# | sc68                                                                 |
# `----------------------------------------------------------------------'

SC68_PACKAGE([
    68000 disassembler/sourcer.
    Support for TOS, sc68 and binary files.
  ])
SC68_SYSTEM
SC68_OPTIONS
SC68_TOOLS
SC68_CHECKS
SC68_WITH([desa68],[yes])
SC68_WITH([file68],[no],[],[sc68/file68.h])

# ,----------------------------------------------------------------------.
# | Standard headers and functions                                       |
# `----------------------------------------------------------------------'
AC_CHECK_HEADERS([stdint.h unistd.h libgen.h getopt.h])
AC_CHECK_FUNCS([basename getopt getopt_long])

# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

AC_CONFIG_FILES([
    Makefile
    package.texi
    sourcer68-${VERSION}.ebuild:sourcer68.ebuild.in
    sourcer68.1.tpl
    sourcer68.spec])
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
AC_MSG_NOTICE([|   file68              : $has_file68 ($file68_ver)])
AC_MSG_NOTICE([+-----------------------])
