#! /bin/sh
#
# Nice plot of average spectrum of all variantes of the ym-2149 engine
#
# by Benjamin Gerard <http://sourceforge.net/users/benjihan>
#

me=$(basename "$0" .sh)
here=$(dirname "$0")
here=$(cd "$dirname"; pwd)
PATH="$here:$PATH"
verb=0

frq=48000
fft=1024
sec=30
typ=""

emsg() {
    echo "$me: $@" 1>&2
}

dcat() {
    sed -e 's/^/>> /' 1>&4
}

dmsg() {
    echo "$@" | dcat
}



usage() {
    cat <<EOF
Usage: $me [OPTION] [--] <input-sc68>

  Draw a nice figure of average spectrum of the ym-2149 engine.

Options:

  -h         Print this message and exit.
  -r <hz>    Set replay rate ($frq)
  -s <size>  FFT size ($fft)
  -t <sec>   Duration ($sec seconds)
  -o <type>  [png|svg]
  -v         Verbose

Require:

  gnuplot - sc68 - dump-average-spectrum.py

EOF
    exit 1
}

###########
# Options #
###########

opts="vhr:s:t:o:"
getopt -Q -n "$me" -o ${opts}  -- "$@" || exit 255
eval set -- `getopt -n "$me" -o  ${opts} -- "$@"`

while [ $# -gt 0 ] && [ "$1" != "--" ]; do
    case "$1" in
	-h) usage;;
	-v) let verb++;;
	-i) typ="";;
	-r) shift; frq="$1";;
	-s) shift; fft="$1";;
	-t) shift; sec="$1";;
	-o) shift; typ="$1";;
    esac
    shift
done
[ $1 = "--" ] && shift

echo $# $@

case $# in
    0) emsg "missing argument. Try -h."; exit 1;;
    1) input="$1"; shift
	if [ ! -r "${input}" ]; then
	    emsg "missing file -- $input" 1>&2
	    exit 2;
	fi
	;;
    *) emsg "too many arguments. Try -h."; exit 1;;
esac

if [ $verb -lt 2 ]; then
    exec 4>/dev/null
else
    exec 4>&1
fi

dmsg "Debug is ON"


###########
# Require #
###########

if [ `basename "$here"` = tools ] &&
    [ -d "$here/../contrib" ]; then
    PATH="$(cd "$here/../contrib" && pwd):$PATH"
fi

which sed     >/dev/null || exit 255
which wc      >/dev/null || exit 255
which sc68    >/dev/null || exit 255
which gnuplot >/dev/null || exit 255
which dump-average-spectrum.py >/dev/null || exit 255

dmsg "Have all required tools"

########################################
# variantes: engine filter color label #
########################################

variantes=(
    "blep none    black   BLEP "
    "orig none    purple  no filter"
    "orig boxcar  green   boxcar filter"
    "orig mixed   blue    mixed filter"
    "orig 1-pole  orange  1-pole filter"
    "orig 2-pole  red     2-pole filter"
)

bname=$(basename "$input")
nude=${bname%.*}
pcms=$(($sec*$frq))

dmsg "nude: $nude"
dmsg "pcms: $pcms"

function do_variant() {
    local  eng="$1" filter="$2" color="$3" label tmp

    dcat<<EOF

=======================================
VARIANT:
 engine : $eng
 filter : $filter
 color  : $color
 s-rate : $frq hz
 fft    : $fft
 time   : $sec" ($pcms pcm)

EOF

    shift 3
    label="$*"
    tmp=`mktemp`

    dmsg "Generate PCM into $tmp"
    sc68 -qq -c -r$frq -linf \
	--ym-engine=$eng --ym-filter=$filter --sc68-debug=0 \
	-- "$input"  |
    dd bs=4 count=$pcms |
    dump-average-spectrum.py /dev/stdin $fft $frq 2>/dev/null | tee $tmp


    wc -c "$tmp" | dcat



    echo $tmp
}

(
    case "x-$typ" in
	x-png | x-svg)
	    cat<<EOF
    set term $typ size 1024, 768
    set output "${nude}.$typ"
EOF
	    ;;
	*)
	    typ=""
	    ;;
    esac

    idx=0
    for var in "${variantes[@]}"; do
	let idx=idx+1
	set -- $var
	cat <<EOF
set style line $idx lt 2 lc rgb "$3" lw 1
EOF
    done

    cat<<EOF
TITLE = "Average Spectrum of '${nude}' @${frq}hz ${fft}pt ${sec} sec"
set multiplot layout 3,2 title TITLE
EOF

    idx=0
    for var in "${variantes[@]}"; do
	let idx=idx+1
	set -- $var
	rgb="$3"
 	tmp=`do_variant "$@" 2>/dev/null`
	shift 3
	if [ -r "$tmp" ]; then
	    cat <<EOF
set key noautotitle title "$@"
plot [0:$((frq/2))] [0:150] "$tmp" with lines ls $idx
EOF
	fi
    done 
    if [ -z "$typ" ]; then
	cat
    else
	cat <<EOF
unset multiplot 
quit
EOF
    fi
    )

# | gnuplot
