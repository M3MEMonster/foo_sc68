#!/bin/sh
#
# Delete all files and directories not under svn source control.
#
# !!!   BEWARE    !!! THIS PROGRAM IS HARMED AND DANGEROUS FOR YOURS FILES
# !!! NO WARRANTY !!! USE WITH CARE AND AT YOUR OWN RISK !!!
#
# Copyleft
#
# $Id: svn-mrproper.sh 138 2010-07-28 22:42:39Z benjihan $

if [ $# = 0 ]; then
    cmd="help"
else
    cmd="$1"; shift 
fi;
action="true"

opt_force=""
opt_verbose=""

for opt in "$@"; do
    case "$opt" in
	-v | --verbose) opt_verbose="-v";;
	-f | --force)   opt_force="-f";;
	*)
	    echo "svn-mrproper: invalid option '$opt'" 1>&2
	    exit 255 ;;
    esac
done

file_type() {
    local type='?'
    if   test -f "$1"; then type='F'
    elif test -h "$1"; then type='L'
    elif test -d "$1"; then type='D'
    elif test -c "$1"; then type='C'
    elif test -S "$1"; then type='S'
    fi
    echo $type
}

action_list() {
    local type=$(file_type "$1") extra=""
    case "$type" in
	D) test -e "$1/.keep" && return 0
	   extra="["$(find "$1" -type f | wc -l)" file(s)]";;
	L) extra="--> "$(readlink "$1");;
    esac
    echo "$type $1 $extra"
}

action_clean() {
    local type=$(file_type "$1")
    case "$type" in
	D)  test -e "$1/.keep" ||
	    rm -r ${opt_verbose} ${opt_force} -- "$1" ;;
	*)  rm ${opt_verbose} ${opt_force} -- "$1" ;;
    esac
}


case "$cmd" in
    --help | -h | help)
	cat <<EOF
Usage: svn-mrproper <COMMAND> [OPTION]

 Clean SVN repository by removing all files and directories not under
 source control.

 Using 'list' command before 'clean' is *LARGELY* encouraged.

 Directory not under source control that content a '.keep' file will
 not be deleted.

 COMMAND:

   help         Print help message.
   list         List file and directories that should be deleted by clean
   clean        Delete everything not in under svn control

 OPTION:

  -f --force    Use '-f' option for 'rm' and 'rmdir' commands
  -v --verbose  Use '-v' option for 'rm' and 'rmdir' commands 

!!!   BEWARE    !!! THIS PROGRAM IS HARMED AND DANGEROUS FOR YOURS FILES
!!! NO WARRANTY !!! USE WITH CARE AND AT YOUR OWN RISK !!!
EOF
	exit 1
	;;
    clean | list)
	action="action_$cmd";;
    *)
	echo "svn-mrproper: invalid command '$cmd'; try --help" 1>&2
	exit 255 ;;
esac

which svn >/dev/null || exit 255
svn info >/dev/null  || exit 255
svn status | grep ^? | sed 's/^?[[:space:]]*\(.*\)$/\1/g' |
while read line; do
    $action "$line" || break
done
