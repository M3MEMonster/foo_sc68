dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20140310 ac_package_extra.m4

# AC_PACKAGE_INFO([DESCRIPTION])
# ------------------------------
# Defines supplemental package info strings
AC_DEFUN_ONCE([AC_PACKAGE_INFO],[
    m4_ifval([$1],
      [
	m4_define([AX_PACKAGE_DESC],
	  [m4_normalize([$1]m4_ifnblank([AC_PACKAGE_URL],
		[ Visit <AC_PACKAGE_URL>]))])
	AC_DEFINE_UNQUOTED(
	  [PACKAGE_DESC],
	  ["AX_PACKAGE_DESC"],
 	  [Package short description])
	AC_SUBST([PACKAGE_DESC],["AX_PACKAGE_DESC"])
	m4_define([AX_PACKAGE_SHORTDESC],
	  m4_bpatsubst(AX_PACKAGE_DESC,[\..*]))
	AC_SUBST([PACKAGE_SHORTDESC],["AX_PACKAGE_SHORTDESC"])
        ])
    m4_ifval([AC_PACKAGE_VERSION],
      [
        m4_define([AX_PACKAGE_VERNUM],patsubst(AC_PACKAGE_VERSION,[[^0-9]],[]))
        AC_DEFINE_UNQUOTED(
	  [PACKAGE_VERNUM],
	  [AX_PACKAGE_VERNUM],
          [Version number])
        AC_SUBST([PACKAGE_VERNUM],[AX_PACKAGE_VERNUM])
        ])
    ])

dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of ac_package_extra.m4
dnl#
dnl# ----------------------------------------------------------------------
