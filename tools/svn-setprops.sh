#! /bin/sh
#
# Time-stamp: <2013-09-27 09:30:26 ben>
#
# Set SVN properties according to file name or extension.
#

svn info >/dev/null || exit 255

# Files eligible for 'Id' keyword property
find . -type f '('  \
    -name '*.[ch]' -o \
    -name '*.in' -o \
    -name '*.m4' -o \
    -name '*.sh' -o \
    -name 'Makefile.am' -o \
    -name 'configure.ac' \
    ')' -execdir svn propset svn:keywords Id {} ';'

# Files eligible for execute access
find . -type f '(' \
    -name '*.sh' -o \
    -name '*.py' \
    ')' -execdir svn propset svn:executable '*' {} ';'
