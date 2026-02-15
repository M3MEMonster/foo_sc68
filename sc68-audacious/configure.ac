dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sc68-audacious
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [sc68-audacious],
  [0.2.0],
  [http://sourceforge.net/projects/sc68/],
  [sc68-audacious],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([sc68-audacious.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
                  no-installman no-installinfo])
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([win32-dll disable-static])

# ,----------------------------------------------------------------------.
# | sc68                                                                 |
# `----------------------------------------------------------------------'

SC68_PACKAGE([Atari ST and Amiga audio plugin for audacious.])
SC68_SYSTEM
SC68_OPTIONS
SC68_TOOLS
SC68_CHECKS
SC68_THREADS
SC68_WITH([sc68],[yes],[sc68_version],[sc68/sc68.h],[libsc68])
SC68_WITH_HEADER([file68],[yes],[sc68/file68.h])

AC_ARG_WITH(
  [audacious-dir],
  [AS_HELP_STRING([--with-audacious-dir],
      [audacious imput plugin location @<:@auto*|user|path@:>@])],
  [],[with_audacious_dir=auto])

AC_MSG_CHECKING([for pkg-config audacious module])
if ! [$]pkgconfig --exists audacious; then
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([unable to configure a required package ... audacious])
else
  audacious_ver=[$]([$]pkgconfig audacious --modversion)
  AC_MSG_RESULT([yes ($audacious_ver)])
fi

audacious_min='2.4.0'
AC_MSG_CHECKING([if audacious version >= $audacious_min]) # need more test on version
if [$]pkgconfig audacious --atleast-version=$audacious_min; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([installed audacious version is too old ($audacious_ver<$audacious_min)])
fi

AM_CONDITIONAL([AUDACIOUS_AT_LEAST_35],
  [[$]pkgconfig audacious --atleast-version=3.5])

# Convert version string to version number 
set -- `echo [$]audacious_ver | sed 's%\.% %g'`
audvnum=0; audmul=10000
while test [$]# -gt 0; do
  let audvnum=audvnum+[$]1*audmul
  let audmul=audmul/100
  shift
done
AC_DEFINE_UNQUOTED([AUDACIOUS_VERNUM],[$audvnum],[audacious version number])

case x"$with_audacious_dir" in

  xuser)
    with_audacious_dir=$HOME/.local/share/audacious/Plugins
    ;;

  xauto)
    with_audacious_dir=`[$]pkgconfig audacious --variable=input_plugin_dir`
    if test x"$with_audacious_dir" = x; then
      with_audacious_dir=`[$]pkgconfig audacious --variable=plugin_dir`/Input
    fi
    if ! test -d "$with_audacious_dir"; then
      AC_MSG_ERROR([Unable to locate audacious input plugins directory. Try --with-audacious-dir.])
    fi
    ;;

  x/*)
    ;;

  *)
    AC_MSG_ERROR([Invalid value for --with-audacious-dir (auto|user|<path>)])
    ;;
esac
AC_SUBST([plugindir],[$]with_audacious_dir)

aud_cflags="[$]([$]pkgconfig audacious --cflags)"
aud_lflags="[$]([$]pkgconfig audacious --libs)"

if test -n "$aud_cflags"; then
  SC68_ADD_FLAGS([LIB_CFLAGS],$aud_cflags)
fi
if test -n "$aud_lflags"; then
  SC68_ADD_FLAGS([LIB_LFLAGS],$aud_lflags)
fi

if [$]pkgconfig [$]glib_pkg --atleast-version=2.32; then
  AC_DEFINE([USE_NEW_GLIB_MUTEX],[1],[Use new mutex init for GLIB>=2.32])
fi


# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

AC_CONFIG_FILES([
    Makefile])
AC_OUTPUT

# ,----------------------------------------------------------------------.
# | Summary                                                              |
# `----------------------------------------------------------------------'

audacious_plugin_dir=`echo $with_audacious_dir | sed 's#^'"$prefix"'#$prefix#'`

SC68_DUMPVARS

AC_MSG_NOTICE([])
AC_MSG_NOTICE([,-----------------------])
AC_MSG_NOTICE([| $PACKAGE_NAME ($PACKAGE_VERSION)])
AC_MSG_NOTICE([|-----------------------])
AC_MSG_NOTICE([|   compile mode        : $compile_mode])
AC_MSG_NOTICE([|   file68 headers      : $has_file68 ($file68_ver)])
AC_MSG_NOTICE([|   libsc68             : $has_sc68 ($sc68_ver)])
AC_MSG_NOTICE([|   audacious           : $audacious_ver ($audvnum)])
AC_MSG_NOTICE([|   plugin-dir          : $audacious_plugin_dir])
AC_MSG_NOTICE([+-----------------------])
