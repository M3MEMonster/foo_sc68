dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sc68-gst
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [sc68-gst],
  [0.1.0],
  [http://sourceforge.net/projects/sc68/],
  [sc68-gst],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([gstsc68.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
                  no-installman no-installinfo])
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([win32-dll])

# ,----------------------------------------------------------------------.
# | sc68                                                                 |
# `----------------------------------------------------------------------'

SC68_PACKAGE([Atari ST and Amiga audio decoder for gstreamer.])
SC68_SYSTEM
SC68_OPTIONS
SC68_TOOLS
SC68_CHECKS
SC68_WITH([sc68],[yes],[sc68_version],[sc68/sc68.h],[libsc68])
SC68_WITH_HEADER([file68],[yes],[sc68/file68.h])

# ,----------------------------------------------------------------------.
# | gstreamer                                                            |
# `----------------------------------------------------------------------'

gst_rel='0.10'                      # which gstreamer release
gst_min='0.10.16'                   # minimal version for this release

AC_ARG_WITH(
  [gst-plugin-dir],
  [AS_HELP_STRING([--with-gst-plugin-dir],
      [where to install the plugin @<:@default=no@:>@])],
  [],[with_gst_plugin_dir=no])

gst_cflags=''; gst_lflags='';
for p in gstreamer gstreamer-base gstreamer-controller; do
  AC_MSG_CHECKING([for pkg-config [$]p ([$]gst_rel) at least [$]gst_min])
  if [$]pkgconfig --atleast-version=[$]gst_min [$]p-[$]gst_rel; then
    AC_MSG_RESULT([yes])
    gst_cflags="[$]gst_cflags [$]([$]pkgconfig [$]p-[$]gst_rel --cflags)"
    gst_lflags="[$]gst_lflags [$]([$]pkgconfig [$]p-[$]gst_rel --libs)"
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([unable to configure a required package ... [$]p-[$]gst_rel >= [$]gst_min])
  fi
done
gst_ver=`[$]pkgconfig gstreamer-[$]gst_rel --modversion`

if test x"$with_gst_plugin_dir" = xno; then
  with_gst_plugin_dir=`[$]pkgconfig gstreamer-[$]gst_rel --variable=pluginsdir`
elif test x"$with_gst_plugin_dir" = xuser; then
  with_gst_plugin_dir="$HOME/.gstreamer-[$]gst_rel/plugins"
fi
AC_SUBST([plugindir],[$]with_gst_plugin_dir)

case x"[$]with_gst_plugin_dir" in
  x/*);;
  *) AC_MSG_ERROR([plugin dir needs to be an absolute path]) ;;
esac

if test -n "[$]gst_cflags"; then
  SC68_ADD_FLAGS([LIB_CFLAGS],[$]gst_cflags)
fi
if test -n "[$]gst_lflags"; then
  SC68_ADD_FLAGS([LIB_LFLAGS],[$]gst_lflags)
fi

enable_gst_typefinder='yes'
AM_CONDITIONAL([TYPEFIND_SC68],[test "x${enable_gst_typefinder}" = 'xyes'])

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
AC_MSG_NOTICE([|   gstreamer           : $gst_rel ($gst_ver)])
AC_MSG_NOTICE([|   plugin-dir          : $with_gst_plugin_dir])
AC_MSG_NOTICE([|   gst type finder     : $enable_gst_typefinder])
AC_MSG_NOTICE([+-----------------------])
