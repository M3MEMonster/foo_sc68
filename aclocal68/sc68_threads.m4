dnl# -*- mode:sh; sh-basic-offset:2; indent-tabs-mode:nil -*-
dnl#
dnl# autoconf macros
dnl#
dnl# (C) 2009-2015 Benjamin Gerard <http://sourceforge.net/users/benjihan>
dnl#
dnl# Distribued under the term of the GPL3+

# serial 20150112 sc68_threads.m4


# SC68_PTHREADS()
# -------------
# How to compile with pthread (posix threads) support
AC_DEFUN([SC68_PTHREADS],[

    dnl # Only support pthread right now.
    AC_CHECK_HEADERS([pthread.h],[],
      [AC_MSG_ERROR([could not locate pthread.h header])])

    sc68_threads=no

    if test "x[$]sc68_threads" = xno; then
        AC_SEARCH_LIBS([pthread_create],[pthread],[sc68_threads=pthread])
    fi

    if test "x[$]sc68_threads" = xno; then
        sc68_threads_cflags="[$]CFLAGS"
        SC68_ADD_FLAG(CFLAGS,-pthread)
        AC_SEARCH_LIBS([pthread_create],[pthread],
                       [
                         sc68_threads=pthread
                         SC68_ADD_FLAG([ALL_CFLAGS],-pthread)
                       ])
        CFLAGS="[$]sc68_threads_cflags"
        unset sc68_threads_cflags
    fi
                   ])
    if test x"[$]sc68_threads" = xno; then
      AC_MSG_ERROR([unable to configure thread support])
    fi
    ]
)

 
# SC68_THREADS()
# ------------
# How to compile with threads support (only POSIX threads ATM).
AC_DEFUN([SC68_THREADS],[SC68_PTHREADS])


dnl# ----------------------------------------------------------------------
dnl#
dnl# End of sc68_threads.m4
dnl#
dnl# ----------------------------------------------------------------------
