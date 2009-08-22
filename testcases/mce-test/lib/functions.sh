#
# Function library: provide common functions
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is released under the GPLv2.
#

setup_path()
{
    export PATH=$ROOT/bin:$PATH
}

script_dir()
{
    local rd=$(dirname "$0")
    (cd $rd; pwd)
}

relative_path()
{
    local len1=${#1}
    local len2=${#2}
    if [ $len1 -eq 0 -o $len1 -ge $len2 -o "${2:0:$len1}" != "$1" ]; then
	die "$2 is not the sub-path of $1!"
    fi
    len1=$((len1 + 1))
    echo "${2:$len1}"
}

die()
{
    echo "DIE: $@"
    echo "DIE: $@" 1>&2
    exit -1
}

driver_prepare()
{
    mkdir -p $WDIR/stamps
}

check_kern_warning_bug()
{
    local f="$1"
    [ -n "$f" ] || die "missing parameter for check_kern_warning"
    grep -e '----\[ cut here \]---' $f > /dev/null || \
	grep -e 'BUG:' $f > /dev/null
}

random_sleep()
{
    local s=$((RANDOM / 13107 + 5))
    sleep $s
}

start_background()
{
    if [ -n "$BACKGROUND" ]; then
	pid_background=$(bash -i -c "$BACKGROUND &>$WDIR/background_log & echo \$!")
	if ! ps -p $pid_background > /dev/null; then
	    die "Failed to start background testing: $BACKGROUND"
	fi
    fi
}

stop_background()
{
    if [ -n "$pid_background" ]; then
	if ! kill -TERM -$pid_background &> /dev/null; then
	    kill $pid_background || true
	fi
    fi
}

filter_fake_panic()
{
    local orig_klog=$1
    local new_klog=$2
    [ $# -eq 2 ] || die "missing parameter for filter_fake_panic"

    local pn
    pn=$(grep -n "Fake kernel panic" $orig_klog | cut -d ':' -f 1 | head -1)
    if [ -z "$pn" ]; then
	cp $orig_klog $new_klog
    else
	sed -n "1,${pn}p" < $orig_klog > $new_klog
    fi
}
