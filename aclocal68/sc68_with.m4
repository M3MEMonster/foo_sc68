dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distributed under the term of the GPL
dnl#

# serial 20140310 sc68_with.m4

# DUMP_SC68_PACKAGE_VARS
# ----------------------
# $1:package var name
# $2:label
m4_define([DUMP_SC68_PACKAGE_VARS],[
    cat <<EOF
==$2==========================================================
 --  $1_has=[$]has_$1
 --  $1_req=[$]$1_req
 --  $1_ver=[$]$1_ver
 --  $1_org=[$]$1_org
 --  $1_fct=[$]$1_fct
 --  $1_hdr=[$]$1_hdr
 --  $1_ccf=[$]$1_ccf
 --  $1_def=[$]$1_def
 --  $1_inc=[$]$1_inc
 --  $1_ldf=[$]$1_ldf
 --  $1_src=[$]$1_src
 --  $1_abs=[$]$1_abs
 --  $1_dir=[$]$1_dir
 --  $1_dbn=[$]$1_dbn
 --  $1_pkg=[$]$1_pkg
 --  LIB_CFLAGS=[$]LIB_CFLAGS
 --  LIB_LFLAGS=[$]LIB_LFLAGS
 --  BIN_CFLAGS=[$]BIN_CFLAGS
 --  BIN_LFLAGS=[$]BIN_LFLAGS
 --  ALL_CFLAGS=[$]ALL_CFLAGS
 --  ALL_LFLAGS=[$]ALL_LFLAGS
 --  PAC_REQUIRES=[$]PAC_REQUIRES
 --  PAC_PRIVREQ=[$]PAC_PRIVREQ
 --  PAC_CONFLICTS=[$]PAC_CONFLICTS
 --  PAC_CFLAGS=[$]PAC_CONFLICTS
 --  PAC_LFLAGS=[$]PAC_LFLAGS
 --  PAC_PRIVLIB=[$]PAC_PRIVLIB
 -- 
 --  LIBS=[$]LIBS
 --  LDFLAGS=[$]LDFLAGS
 --  CFLAGS=[$]CFLAGS
 --  CPPFLAGS=[$]CPPFLAGS
======================================================================
EOF
    ])


# UNSET_SC68_PACKAGE_VARS
# -----------------------
# $1:var name
m4_define([UNSET_SC68_PACKAGE_VARS],[
    unset $1_req $1_ver $1_fct $1_hdr $1_def
    unset $1_inc $1_ccf $1_ldf $1_src $1_abs
    unset $1_dir $1_dbn $1_pkg $1_org
    ])

# COPY_SC68_PACKAGE_VARS
# ----------------------
# $1:dst var name
# $2:src var name
m4_define([COPY_SC68_PACKAGE_VARS],[
    $1_req="[$]$2_req"; $1_ver="[$]$2_ver"
    $1_fct="[$]$2_fct"; $1_hdr="[$]$2_hdr"
    $1_def="[$]$2_def"; $1_inc="[$]$2_inc"
    $1_ccf="[$]$2_ccf"; $1_ldf="[$]$2_ldf"
    $1_src="[$]$2_src"; $1_abs="[$]$2_abs"
    $1_dir="[$]$2_dir"; $1_dbn="[$]$2_dbn"
    $1_pkg="[$]$2_pkg"; $1_org="[$]$2_org"
    ])

# RESET_SC68_PACKAGE_VARS
# -----------------------
# $1:package var name
# $2:required           (yes|no)
# $3:function           (function to link)
# $4:header             (header file to test)
# $5:dirbasename
m4_define([RESET_SC68_PACKAGE_VARS],[
    has_$1="no"                 # package found yes/no ?
    $1_req="$2"                 # required yes/no ?
    $1_ver="n/a"                # package version
    $1_org="n/a"                # package origine (how it was detected)
    $1_ccf=""                   # package cflags
    $1_def=""                   # package cppflags define (-D -U)
    $1_inc=""                   # package cppflags include (-I)
    $1_ldf=""                   # package extra library paths (-L -l)
    $1_src=""                   # package source (rel. to build)
    $1_abs=""                   # package source absolute
    $1_dir=""                   # package build (rel. to build)
    $1_fct="m4_default([$3],[$1_version])" # function to search
    $1_hdr="m4_default([$4],[$1.h])"       # header to search
    $1_dbn="m4_default([$5],[$1])"         # dir basename
    $1_pkg="$1"                            # package name (eg for pkg-config)
    ])


# DO_SC68_PACKAGE
# ---------------
# $1:package_var_name  (pack_age)
# $2:package-name      (dirname)
# $3:PACKAGE_DEF_NAME  (PACK_AGE)
# $4:required          (yes/no)
# $5:function          ('-' => ignore; '+' => header only)
# $6:header            ('-' => ignore)
# $7:dirbasename
m4_define([DO_SC68_PACKAGE],[
    # Declare with options 
    AC_ARG_WITH(
      [$2],
      [AS_HELP_STRING([--with-$2],[which $2 package])],
      [],[with_$1=check])

    # Declare with headers
    AC_ARG_WITH(
      [$2],
      [AS_HELP_STRING([--with-$2-headers],[Locate $2 package headers])],
      [],[unset with_$1_headers])

    # Declare with sources
    AC_ARG_WITH(
      [$2],
      [AS_HELP_STRING([--with-$2-sources],[Locate $2 package sources])],
      [],[unset with_$1_sources])
    
    # Reset all package_vars
    RESET_SC68_PACKAGE_VARS([$1],[$4],[$5],[$6],[$7])
    $1_pkg="$2"

    # Verify --with-<package>-sources directory
    if test "x-[$]{with_$1_sources-set}" != "x-set"; then
      AC_MSG_CHECKING([$2 source directory])
      if ! test -d "[$]{with_$1_sources}"; then
        unset with_$1_sources
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([invalid source directory specified for $2])
      fi
      AC_MSG_RESULT([yes ([$]{with_$1_sources})])

      if test "x-[$]{with_$1_headers-set}" = "x-set"; then
        if test -d "[$]{with_$1_sources}/includes"; then
          with_$1_headers="[$]{with_$1_sources}/includes"
        elif  test -d "[$]{with_$1_sources}/include"; then
          with_$1_headers="[$]{with_$1_sources}/include"
        else
          with_$1_headers="[$]{with_$1_sources}"
        fi
      fi
    fi

    # Verify --with-<package>-headers directory
    if test "x-[$]{with_$1_headers-set}" != "x-set"; then
      AC_MSG_CHECKING([$2 header directory])
      if ! test -d "[$]{with_$1_headers}"; then
        unset with_$1_headers
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([invalid headers directory specified for $2])
      fi
      AC_MSG_RESULT([yes ([$]{with_$1_sources})])
    fi
    
    # Set actions
    set -- [$](echo [$]with_$1 | sed "s/,/ /g")
    if test "x[$][1]" = "xcheck"; then
      # Default actions
      if test "x-[$]{with_$1_sources-set}" != "x-set"; then
        # --with-package-source has been set: configure source only
        set -- source
        _$1_src="[$]{with_$1_sources}"
      else
        set -- bundle source system
      fi
    fi

    while test [$]# -gt 0; do
      COPY_SC68_PACKAGE_VARS([_$1],[$1])
      has_$1=no 
      _$1_org="[$]1"
      _$1_wth="[$]1"

      case "x-[$]1" in
        
        # YES or NO: no check; assuming everything is configured.
	x-no | x-yes)
          has_$1="[$]1"; _$1_org='set'; break ;;

        # bundle: source package inside source tree; SRCDIR/PACKAGE
	x-bundle | x-source)
  	  _$1_ver="[$]1"
  	  AC_MSG_CHECKING([for [$]_$1_ver $2])
          _$1_src="[$]srcdir/../[$]_$1_dbn"
          test "x-[$]_$1_ver" = "x-bundle" &&
          _$1_src="[$]srcdir/[$]_$1_dbn"

 	  if ! _$1_abs=[$](cd "[$]_$1_src" 2>/dev/null && pwd); then
            _$1_src=""
          elif test -e "[$]_$1_abs/configure.gnu" ||
            test -e "[$]_$1_abs/configure"; then
            has_$1='maybe'
            test "x[$]_$1_hdr" = "x-" && has_$1=yes
            if test "x-[$]_$1_ver" = "x-bundle"; then
              _$1_dir='[$](top_builddir)/$2'
              _$1_src='[$](top_srcdir)/'"[$]_$1_dbn"
            else
              _$1_dir='[$](top_builddir)/../'"[$]_$1_dbn"
              _$1_src='[$](top_srcdir)/../'"[$]_$1_dbn"
            fi
	    _$1_ccf="-I[$]_$1_src -I[$]_$1_dir"
            #_$1_ldf="-l$1"   # $$$ Or not ...
          fi

 	  AC_MSG_RESULT([$]has_$1)

          if test "x-[$]has_$1" = "x-maybe"; then
            ac_wp_CPPFLAGS="[$]CPPFLAGS"
            CPPFLAGS="-I[$]_$1_abs [$]CPPFLAGS"
            AC_CHECK_HEADERS([[$]_$1_hdr],[has_$1=yes],[has_$1=no])
            CPPFLAGS="[$]ac_wp_CPPFLAGS"; unset ac_wp_CPPFLAGS
          fi
          
 	  ;;
        
        x-static)
          AC_MSG_ERROR([Deprecated use --with-$1=static; use --enable-sc68-static=lib])
          ;;

        # system: installed package
        x-system)
          AC_PATH_PROG([pkgconfig],["pkg-config"],["false"])
          unset _$1_pkg
          if test x"[$]enable_sc68_static" != xno ||
            test x"[$]_$1_wth" = xstatic; then
            pkgconfig="[$]pkgconfig --static"
          fi
	  AC_MSG_CHECKING([for pkg-config $2 module])
          if ! [$]pkgconfig --exists "$2"; then
            AC_MSG_RESULT([no])
	    AC_MSG_CHECKING([for pkg-config lib$2 module])
            if ! [$]pkgconfig --exists "lib$2"; then
              AC_MSG_RESULT([no])
	      AC_MSG_CHECKING([for pkg-config $2lib module])
              if ! [$]pkgconfig --exists "$2lib"; then
                AC_MSG_RESULT([no])
              else
                _$1_pkg=$2lib
              fi
            else
              _$1_pkg=lib$2
            fi
          else
            _$1_pkg=$2
          fi

          if test s[$]{_$1_pkg+et} = set; then
            # use pkg-config
            _$1_org='pkg-config'
            _$1_ver=[$]([$]pkgconfig "[$]_$1_pkg" --modversion||echo installed)
            _$1_ccf=[$]([$]pkgconfig "[$]_$1_pkg" --cflags)
            if test "x[$]_$1_fct" != 'x+'; then
              _$1_ldf=[$]([$]pkgconfig "[$]_$1_pkg" --libs)
            fi
            AC_MSG_RESULT([yes ([$]_$1_ver)])
            $1_config=false
          fi

          if test s[$]{$1_config+et} != set; then
            AC_PATH_PROG([$1_config],["$2-config"],["false"])
            if test "[$]$1_config" != false; then
              _$1_pkg="$2"
            else
              unset $1_config
            fi
          fi
          if test s[$]{$1_config+et} != set; then
            AC_PATH_PROG([$1_config],["lib$2-config"],["false"])
            if test "[$]$1_config" != false; then
              _$1_pkg="lib$2"
            else
              unset $1_config
            fi
          fi
          if test s[$]{$1_config+et} != set; then
            AC_PATH_PROG([$1_config],["$2lib-config"],["false"])
            if test "[$]$1_config" != false; then
              _$1_pkg="$2lib"
            fi
          fi

          if test "[$]{$1_config}" != false; then
            _$1_org='config-script'
            _$1_ver=[$]([$]$1_config --version || echo installed)
            _$1_ccf="[$]_$1_ccf [$]([$]$1_config --cflags)"
            if test "x[$]_$1_fct" != 'x+'; then
              _$1_ldf="[$]_$1_ldf [$]([$]$1_config --libs)"
            fi
          fi

          for ac_wp_var in : [$]_$1_ccf; do
            case [$]ac_wp_var in
              :) ;;
              -I* )       _$1_inc="[$]_$1_inc [$]ac_wp_var" ;;
              -D* | -U*)  _$1_def="[$]_$1_def [$]ac_wp_var" ;;
            esac
          done; unset ac_wp_var
          _$1_inc=[$](echo [$]_$1_inc)
          _$1_def=[$](echo [$]_$1_def)

          for ac_wp_var in CPPFLAGS CFLAGS LDFLAGS LIBS; do
            eval ac_wp_[$]ac_wp_var=\"[\$][$]ac_wp_var\"
          done; unset ac_wp_var

          if test x"[$]_$1_wth" = xstatic; then
            if test x"[$]GCC" != xyes; then
              AC_MSG_ERROR(
                [Can only link partial static library with gcc])
            else
              _$1_org="[$]_$1_org (static)"
              LDFLAGS="-static [$]LDFLAGS"
            fi
          fi

          _$1_lib=''
          for ac_wp_var in : [$]_$1_ldf; do
            case [$]ac_wp_var in
              -L*)
                LDFLAGS="[$]LDFLAGS [$]ac_wp_var" ;;
              -l$2) ;;
              -l*) _$1_lib="[$]_$1_lib [$]ac_wp_var" ;;
            esac
          done; unset ac_wp_var

          CPPFLAGS="[$]_$1_def [$]_$1_inc [$]CPPFLAGS"

          if test "x[$]_$1_hdr" = "x-"; then
            has_$1=maybe
          else
            AC_CHECK_HEADERS([$][_$1_hdr],[has_$1=maybe])
          fi
          
          if test "x[$]has_$1" = "xmaybe"; then
            case "x[$]_$1_fct" in
              x- | x+) has_$1=yes ;;
              *)
                AC_SEARCH_LIBS(
                  [[$]_$1_fct],
                  [$2],
                  [has_$1=yes; SC68_ADD_FLAG(_$1_ldf,-l$2)],
                  [has_$1=no],
                  [[$]_$1_lib])
                ;;
            esac
          fi

          # ??? Should we restore LIBS ?
          for ac_wp_var in CPPFLAGS CFLAGS LDFLAGS LIBS; do
            eval [$]ac_wp_var=\"[\$]ac_wp_[$]ac_wp_var\"
          done; unset ac_wp_var
	  ;;
        
	*)
          ;;
      esac
      
      if test "x[$]{has_$1}" = 'xyes'; then
        COPY_SC68_PACKAGE_VARS([$1],[_$1])
        break
      elif test "x[$]1" = 'xno'; then
        break
      fi
      shift
    done
    UNSET_SC68_PACKAGE_VARS([_$1])
    
    dnl # TODO: PAC_PRIVREQ and PAC_PRIVLIB are used for private
    dnl #       library in pkgconfig files:
    dnl #
    dnl #       Libraries which are not exposed through our library,
    dnl #       but are needed in the case of static linking. This
    dnl #       differs from Requires.private: in that it references
    dnl #       libraries that do not have package files installed.
    dnl #
    dnl # !!! THIS WHOLE THING IS A MESS WE NEED TO SORT OUT SOMETIME !!!

    if test "x[$]has_$1" = 'xyes'; then
      case x-"[$]$1_org" in
        x-yes)
          ;;
        x-bundle | x-source)
          SC68_ADD_FLAGS([ALL_CFLAGS],[$]$1_ccf)
          if test "x[$]$1_fct" != "x+"; then
            if test "x-[$]$1_org" = "x-bundle"; then
              AC_CONFIG_SUBDIRS([$2])
            else
              SC68_ADD_FLAG([PAC_REQUIRES],[$2])
            fi
          fi
          ;;
        x-system | x-config-script)
          SC68_ADD_FLAGS([ALL_CFLAGS],[$]$1_ccf)
          SC68_ADD_FLAGS([ALL_LFLAGS],[$]$1_ldf)
          SC68_ADD_FLAGS([PAC_LFLAGS],[$]$1_ldf)
          ;;
        x-pkg-config)
          SC68_ADD_FLAGS([ALL_CFLAGS],[$]$1_ccf)
          SC68_ADD_FLAGS([ALL_LFLAGS],[$]$1_ldf)
          SC68_ADD_FLAG([PAC_REQUIRES],[$]$1_pkg)
          ;;
        *'(static)')
          SC68_ADD_FLAGS([ALL_CFLAGS],[$]$1_ccf)
          $1_ldf="[$](echo -Wl -dn [$]$1_ldf -dy|sed 's/ /,/g')"
          SC68_ADD_FLAGS([ALL_LFLAGS],[$]$1_ldf)
          ;;
        *)
          AC_MSG_ERROR([Internal error: invalid value for $1_org '[$]$1_org'])
          ;;
      esac
    elif test "x[$]$1_req" = 'xyes'; then
      AC_MSG_ERROR([unable to configure a required package ... $1])
    fi
    
    AM_CONDITIONAL([SOURCE_]$3,
      [test "x[$]$1_src" != "x" && test "x[$]_$1_fct" != "x+"])

    if test "x[$]has_$1" = "xyes"; then
      AC_DEFINE([USE_]$3,[1],[Using $2 library])
      AC_SUBST($1[_abs])
      AC_SUBST($1[_def])
      AC_SUBST($1[_dir])
      AC_SUBST($1[_inc])
      AC_SUBST($1[_ldf])
      AC_SUBST($1[_ccf])
      AC_SUBST($1[_src])
      AC_SUBST($1[_ver])
    fi
    if test X[$]enable_sc68_dumpvars = Xyes; then
      DUMP_SC68_PACKAGE_VARS([$1],[ END ])
    fi
    ])


# SC68_WITH([PACKAGE],[REQ],[FUNC],[HEADER],[DIRNAME])
# ----------------------------------------------------
# 
AC_DEFUN([SC68_WITH],
  [
    DO_SC68_PACKAGE(
      AS_TR_SH(m4_strip[$1]),
      m4_strip([$1]),
      AS_TR_CPP(m4_strip[$1]),
      m4_if([yes],m4_default(m4_strip([$2]),[no]),[yes],[no]),
      m4_strip([$3]),
      m4_strip([$4]),
      m4_strip([$5]))
    ])

# SC68_WITH_HEADER([PACKAGE],[REQ],[HEADER],[DIRNAME])
# ----------------------------------------------------
# 
AC_DEFUN([SC68_WITH_HEADER],
  [
    DO_SC68_PACKAGE(
      AS_TR_SH(m4_strip[$1]),
      m4_strip([$1]),
      AS_TR_CPP(m4_strip[$1]),
      m4_if([yes],m4_default(m4_strip([$2]),[no]),[yes],[no]),
      [+],
      m4_strip([$3]),
      m4_strip([$4]))
    ])


dnl# ----------------------------------------------------------------------
dnl#
dnl# End Of sc68_with.m4
dnl#
dnl# ----------------------------------------------------------------------
