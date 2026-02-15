#! /bin/sh
#
# Run this script after fresh update. It should do all autotools and
# buddies required before you can configure and compile this package.
#
# this file is part of the sc68 project <http://sc68.atari.org/>
#
# Copyright (C) 2007-2015 Benjamin Gerard
#


me="svn-bootstrap.sh"
ln_s="ln -sf --"
cp_r="cp -R --"
vifs="-vifs"
linking="Linking"

# Display informationnal message
msg() {
    echo "$@"
}

# Display error message
error() {
    echo "$me: $*" 1>&2
    return 1
}

# Display error message and exit
fatal() {
    error "$@"
    exit 127
}

while [ $# -gt 0 ]; do
    case x"$1" in
	x-h|x--usage|x--help)
	    cat <<EOF
Usage: $me [no-links]

  Setup autotools and others required steps to setup sc68 source tree.

EOF
	    exit 0
	    ;;
	xno-link* | xnl)
	    ln_s="${cp_r}"
	    vifs="-vif"
	    linking="Copying"
	    ;;
	*)
	    fatal "invalid argument -- '$1'"
	    ;;
    esac
    shift
done    

# $1:tool  $2:what-if-not
testtool() {
    local tool="$1"
    msg "Checking for tool '${tool}'"
    if ! which "$tool" >/dev/null 2>/dev/null; then
	if [ -n "$2" ]; then
	    error "$2"
	else
	    error "missing '${tool}'"
	fi
	return 1
    fi
}

# $1: file  $2: action
testfile() {
    local file="$1" action="$2"
    msg "Checking for file '${file}'"
    if ! test -f "${file}"; then
	${action:-error} "Missing file '${file}'"
	return 1
    fi
    return 0
}

# $1: file  $2: action
testlink() {
    local file="$1" action="$2"
    msg "Checking for symbolic link '${file}'"
    if ! test -f "${file}" && ! test -h "${file}"; then
	${action:-error} "Missing symbolic link '${file}'"
	return 1
    fi
    return 0
}

# $1: dir  $2: action
testdir() {
    local dir="$1" action="$2"
    msg "Checking for directory '${dir}'"
    if ! (cd "${dir}"); then
	${action:-error} "Invalid directory '${dir}'"
	return 1
    fi
    return 0
}

# $1: source  $2: destination
ln_or_cp() {
    msg "${linking} '$1' -> '$2'"
    $ln_s "$1" "$2" ||
    $cp_r "$1" "$2"
}

# $1: dir  $2: --force
bootstrap_dir() {
    local dir="$1" m4file=ac_package_extra.m4 force='' 
    local test skip obj src lnk dst

    msg "Bootstrapping directory '${dir}'"

    test "$2" = '--force' && force='yes'

    testfile "${dir}/../m4/${m4file}" || return 1
    testfile "${dir}/configure.ac"    || return 1
    testfile "${dir}/Makefile.am"     || return 1
    testfile "${dir}/../AUTHORS"      || return 1
    
    for obj in m4 AUTHORS README; do
	test=false
	dst="${dir}/${obj}"
	if test -e "${dir}/${obj}-${dir}"; then
	    src="${dir}/${obj}-${dir}"
	    lnk="${obj}-${dir}"
	elif test -f "${dir}/${obj}"; then
	    msg "keeping ${dir}/${obj}"
	    continue
	elif test -e "${obj}"; then
	    src="${obj}"
	    lnk="../${obj}"
	else
	    fatal "'${obj}' is missing in action"
	fi
	
	if test -e "${dst}"; then
	    if test -d "${src}"; then
		test=testdir
		if ! test -d "${dst}"; then
		    fatal "'${dst}' exists but is not a directory"
		fi
	    elif test -f "${src}"; then
		test=testfile
		if ! test -f "${dst}"; then
		    fatal "'${dst}' exists but is not a file"
		elif ! diff "${dst}" "${src}" >/dev/null 2>/dev/null
		then
		    if test x$force = xyes; then
			rm -- "${dst}"
		    else
			fatal "'${dst}' exists but differs (--force to overwrite)"
		    fi
		fi
	    else
		fatal "'${src}' exists but is neither a file or a directory"
	    fi
	fi

	test -e "${dst}" || ( cd "${dir}" && ln_or_cp "${lnk}" "${obj}" )
	$test "${dst}" fatal
    done
    return 0;
}

######################################################################

# Test some required tools
testtool help2man    "missing help2man -- install package 'help2man'"
testtool texinfo2man "missing textinfo2man -- compile and install 'tools/texinfo2man.c'"
testtool od

# Test for aclocal68 directory
testdir aclocal68 fatal

# In the top source dir create a m4 directory filled with invidual
# symbolic links to aclocal68 files. All sub directories only link
# this m4 directory.
mkdir -p m4 && testdir m4 fatal
(
    cd m4 &&
    for l in ../aclocal68/*.m4; do
	ln_or_cp "$l" $(basename "$l")
    done
)

# Bootstrap all sub-directories.
dirs="as68 desa68 file68 info68 unice68 file68 sourcer68 libsc68"
dirs="$dirs sc68 mksc68 sc68-gst sc68-audacious sc68-vlc sc68-doc"
err=0
for dir in ${dirs}; do
    bootstrap_dir ${dir} "$@" || let err=err+1
done

if test ${err} -ne 0; then
    fatal "${err} error(s): Not running autoreconf"
fi

# No error runs autoreconf to create missing files.
msg "Running autoreconf ${vifs}"
autoreconf ${vifs}
