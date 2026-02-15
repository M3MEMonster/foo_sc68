dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20141003 sc68_enable_dumpvars.m4

# SC68_ENABLE_DUMPVARS()
# -----------------------------------------------------------------
# Defines --enable-sc68-dumpvars option
AC_DEFUN_ONCE([SC68_ENABLE_DUMPVARS],[
    AC_ARG_ENABLE(
      [sc68-dumpvars],
      [AS_HELP_STRING([--enable-sc68-dumpvars],
	  [dump compile VARS @<:@default=no@:>@])],
      [],[enable_sc68_dumpvars=no])
    SC68_ENABLE_THIS([enable_sc68_dumpvars])
  ])

# SC68_DUMPVARS()
# -----------------------------------------------------------------
# Dump compile and such variables
AC_DEFUN_ONCE([SC68_DUMPVARS],[
    if test x"[$]enable_sc68_dumpvars" = xyes; then
      echo "/----------------------------------------"
      echo "| Dump [VARS] [$]PACKAGE_NAME"
      echo "+----------------------------------------"
      for my_dv in \
        [LIBTOOL] \
        [LIBTOOLFLAGS] \
        [AM_LIBTOOLFLAGS] \
        [CPP] \
        [CPPFLAGS] \
        [AM_CPPFLAGS] \
        [DEFS] \
        [DEFAULT_INCLUDES] \
        [INCLUDES] \
        [CC] \
        [CFLAGS] \
        [AM_CFLAGS] \
        [LD] \
        [CCLD] \
        [LDFLAGS] \
        [AM_LDFLAGS] \
        [LIBS] \
        [ALL_CFLAGS] \
        [ALL_LFLAGS] \
        [LIB_CFLAGS] \
        [LIB_LFLAGS] \
        [BIN_CFLAGS] \
        [BIN_LFLAGS] \
        [PAC_CFLAGS] \
        [PAC_LFLAGS] \
        [PAC_PRIVLIB] \
        [PAC_REQUIRES] \
        [PAC_PRIVREQ] \
        [PAC_CONFLICTS]
      do
        printf "| %-13s = %s\n" "[$]my_dv" "$(eval echo '[$]'[$]my_dv)"
      done
      echo "\----------------------------------------"

    fi
  ])


dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_enable_dumpvars.m4
dnl#
dnl# ----------------------------------------------------------------------
