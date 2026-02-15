dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for mksc68
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [mksc68],
  [20140815],
  [http://sourceforge.net/projects/sc68/],
  [mksc68],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([mksc68.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
                  no-installman no-installinfo])
AC_CONFIG_HEADERS([config.h])
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([win32-dll])

# ,----------------------------------------------------------------------.
# | sc68                                                                 |
# `----------------------------------------------------------------------'

SC68_PACKAGE([sc68 file toolbox.])
SC68_SYSTEM
SC68_OPTIONS
SC68_TOOLS
SC68_CHECKS
SC68_WITH([sc68],[yes],[sc68_version],[sc68/sc68.h],[libsc68])
#SC68_WITH([file68],[yes],[],[sc68/file68.h])
SC68_WITH_HEADER([file68],[yes],[sc68/file68.h])
SC68_WITH_HEADER([unice68],[yes],[unice68.h])

# ,----------------------------------------------------------------------.
# | Standard headers and functions                                       |
# `----------------------------------------------------------------------'

AC_CHECK_HEADERS([stdio.h stdlib.h stdarg.h stdint.h string.h])
AC_CHECK_HEADERS([unistd.h io.h fcntl.h time.h])
AC_CHECK_HEADERS([sys/socket.h sys/ioctl.h netdb.h])

# ,----------------------------------------------------------------------.
# | Extra libraries                                                      |
# `----------------------------------------------------------------------'
AC_SEARCH_LIBS([readline],[readline],
  [AC_CHECK_HEADERS([readline/readline.h readline/history.h])])

SC68_THREADS
if test x"[$]ac_cv_header_sys_socket_h" != x"yes"; then
  AC_CHECK_HEADERS([winsock2.h winsock.h],[break])
  if test x"[$]ac_cv_header_winsock2_h" = x"yes"; then
    SC68_ADD_FLAG([LIBS],[-lws2_32])
  elif test x"[$]ac_cv_header_winsock_h" = x"yes"; then
    SC68_ADD_FLAG(LIBS],[-lwsock32])
  fi
fi

AC_CHECK_FUNCS([malloc free])
AC_CHECK_FUNCS([strdup strncasecmp stpcpy])
AC_CHECK_FUNCS([vsprintf vsnprintf vasprintf])
AC_CHECK_FUNCS([hstrerror])
AC_CHECK_FUNCS([_isatty isatty _fileno fileno])

# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

SC68_DO_ALLSTATIC

AC_CONFIG_FILES([
    Makefile
    mksc68-${VERSION}.ebuild:mksc68.ebuild.in
    mksc68.1.tpl
    mksc68.spec
    package.texi])
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
AC_MSG_NOTICE([|   file68 headers      : $has_file68 ($file68_ver)])
AC_MSG_NOTICE([|   libsc68             : $has_sc68 ($sc68_ver)])
AC_MSG_NOTICE([+-----------------------])
