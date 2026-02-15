dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# the "configure" script is made from this by running GNU "autoconf"
dnl#
dnl# Configuration script for sc68-doc
dnl#
dnl# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#

# ,----------------------------------------------------------------------.
# | Autotools and Package                                                |
# `----------------------------------------------------------------------'

AC_INIT(
  [sc68-doc],
  [20140801],
  [http://sourceforge.net/projects/sc68/],
  [sc68-doc],
  [http://sc68.atari.org])
AC_CONFIG_SRCDIR([sc68_doc.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.11 silent-rules dist-xz
                  no-installman no-installinfo])
AM_MAINTAINER_MODE

# ,----------------------------------------------------------------------.
# | Package                                                              |
# `----------------------------------------------------------------------'

AC_PACKAGE_INFO(
  [doc68 is sc68 documentation project. It is part of the sc68 project.],
  [http://sc68.atari.org])

# ,----------------------------------------------------------------------.
# | Maintainer mode                                                      |
# `----------------------------------------------------------------------'

AS_IF([test "x${enable_maintainer_mode}" = "xyes"],
  [],
  [])

AC_PROG_MAKE_SET
AC_PATH_PROG([doxygen],[doxygen],[false])
AS_IF([test "x[$]doxygen" = xfalse],
  [AC_MSG_ERROR([doxygen is required. Visit http://www.doxygen.org])])

# ,----------------------------------------------------------------------.
# | Output                                                               |
# `----------------------------------------------------------------------'

doxfile='sc68-doc.dox'
AC_SUBST([doxfile])

mydocdir=`cd "${srcdir}/.." && pwd`

for dir in sc68-doc sc68-doc/sc68_doc.h desa68 unice68 file68/sc68 \
  libsc68/sc68 libsc68/io68 libsc68/emu68 libsc68/dial68 \
  sc68-dshow/sc68-splitter
do
  AC_MSG_CHECKING([whether [$]dir exists])
  if test -e "[$]{mydocdir}/[$]{dir}"; then
    INPUTS="[$]{INPUTS} [$]{mydocdir}/[$]{dir}"
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
done

AC_SUBST([INPUTS])
AC_CONFIG_FILES([
    gen-doxcfg.sh
    doc-filter.sh
    sc68-doc.spec
    sc68-doc-${VERSION}.ebuild:sc68-doc.ebuild.in
    Makefile])
AC_OUTPUT

# ,----------------------------------------------------------------------.
# | Summary                                                              |
# `----------------------------------------------------------------------'
  
AC_MSG_NOTICE([])
AC_MSG_NOTICE([,-----------------------])
AC_MSG_NOTICE([| $PACKAGE_NAME ($PACKAGE_VERSION)])
AC_MSG_NOTICE([+-----------------------])
AC_MSG_NOTICE([])
