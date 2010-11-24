#
# MCE library: provide MCE specific functions
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is released under the GPLv2.
#

extract_mce_from_log()
{
    [ $# -eq 2 ] || die "missing parameter for extract_mce_from_log"
    local log="$1"
    local outf="$2"

    sed '1,/HARDWARE ERROR/d' "$log" | \
	mcelog --no-dmi --dump-raw-ascii --ascii > "$outf"
}

mce_reformat()
{
    [ $# -eq 2 ] || die "missing parameter for mce_reformat"
    local org="$1"
    local outf="$2"

    mce-inject --dump "$org" > "$outf"
}

mce_reformat_for_cmp()
{
    local inf="$1"
    local outf="$2"
    local removes="$3"

    local tmpf=$WDIR/mce_reformat_for_cmp

    mce-inject --dump "$inf" > $tmpf

    if [ -n "$removes" ]; then
	for remove in $removes; do
	    sed "/$remove/d" -i $tmpf
	done
    fi

    cat $tmpf | tr '\n' '#' | sed '1,$s/##/\n/g' | \
	grep -v '#STATUS 0x0#' | \
	grep -v '#STATUS 0x800000000000000#' | sort > "$outf"
}

mce_cmp()
{
    [ $# -eq 3 ] || die "missing parameter for mce_cmp"
    local m1="$1"
    local m2="$2"
    local removes="$3"

    local tmpf1=$WDIR/mce_cmp_1
    local tmpf2=$WDIR/mce_cmp_2

    mce_reformat_for_cmp "$m1" $tmpf1 "$removes"
    mce_reformat_for_cmp "$m2" $tmpf2 "$removes"
    diff $tmpf1 $tmpf2 > /dev/null
}

get_mcelog_from_dev()
{
    [ $# -eq 1 ] || die "missing parameter for get_mcelog_from_dev"
    local mcelog_result="$1"
    if mcelog --dump-raw-ascii > "$mcelog_result"; then
	true
    else
	echo "  Failed: can not get mce log from /dev/mcelog"
    fi
}

# extract mcelog from kernel log
get_mcelog_from_klog()
{
    [ $# -eq 2 ] || die "missing parameter for get_mcelog_from_klog"
    local klog="$1"
    local mcelog_result="$2"
    if [ -f "$klog" ] && extract_mce_from_log "$klog" "$mcelog_result"; then
	true
    else
	echo "  Failed: Can not extract mcelog from console log"
    fi
}

mcelog_filter()
{
    [ $# -eq 2 ] || die "missing parameter for mcelog_filter"
    local inf="$1"
    local pat="$2"

    mce-inject --dump "$inf" | tr '\n' '#' | sed '1,$s/##/\n/g' | \
	grep -e "$pat"
}

chk_gcov()
{
    if [ -z "$GCOV" ]; then
	return 1
    fi

    if [ -f /sys/kernel/debug/gcov/reset ] && which gcov > /dev/null; then
	return 0
    else
	return 1
    fi
}

reset_gcov()
{
    if [ -z "$GCOV" ]; then
	return
    fi
    case $GCOV in
	copy)
	    echo 1 > /sys/kernel/debug/gcov/reset
	    ;;
	dump)
	    true;
	    ;;
	*)
	    echo "  Failed: can not reset gcov, invalid GCOV=$GCOV"
	    return
	    ;;
    esac
}

get_gcov()
{
    [ $# -eq 1 ] || die "missing parameter for get_gcov"
    local src_path=$1
    local src_fn=$(basename $src_path)
    local src_dir=$(dirname $src_path)
    if [ -z "$GCOV" ]; then
	return
    fi
    local abs_dir=$(cd -P $KSRC_DIR/$src_dir; pwd)
    case $GCOV in
	copy)
	    for f in /sys/kernel/debug/gcov/$abs_dir/*.gc*; do
		bf=$(basename $f)
		cat $f > $abs_dir/$bf
	    done
	    ;;
	dump)
	    true
	    ;;
	*)
	    echo "  Failed: can not get gcov path, invalid GCOV=$GCOV"
	    return
	    ;;
    esac
    if ! (cd $KSRC_DIR; gcov -o $src_dir $src_fn &> /dev/null) || \
	! [ -s $KSRC_DIR/$src_fn.gcov ]; then
	echo "  Failed: can not get gcov graph"
	return
    fi
    cp $KSRC_DIR/$src_fn.gcov $RDIR/$this_case
}

reset_severity_cov()
{
    echo 1 > /sys/kernel/debug/mce/severities-coverage
}

get_severity_cov()
{
    local sev_cor=/sys/kernel/debug/mce/severities-coverage
    if [ ! -f $sev_cor ]; then
	echo "  Failed: can not get severities_coverage"
	return
    fi
    cp $sev_cor $RDIR/$this_case
}

verify_klog()
{
    [ $# -eq 1 ] || die "missing parameter for verify_klog"
    local klog="$1"
    if [ -f "$klog" ]; then
	if check_kern_warning_bug "$klog"; then
	    echo "  Failed: kernel warning or bug during MCE"
	else
	    echo "  Passed: No kernel warning or bug"
	fi
    else
	echo "  Failed: no kernel log"
    fi
}

verify_panic_via_klog()
{
    [ $# -eq 2 ] || die "missing parameter for verify_panic"
    local klog="$1"
    local mce_panic="$2"
    if [ ! -f "$klog" ]; then
	echo "  Failed: no kernel log for checking panic"
	return -1
    fi

    if grep "panic" "$klog" | grep "$mce_panic" > /dev/null; then
	echo "  Passed: correct panic"
    else
	echo "  Failed: uncorrect panic, expected: $mce_panic"
    fi
}

verify_timeout_via_klog()
{
    [ $# -eq 1 ] || die "missing parameter for verify_timeout"
    local klog="$1"
    if [ ! -f "$klog" ]; then
	echo "  Failed: No kernel log for checking timeout"
	return -1
    fi

    if grep "Some CPUs didn't answer in synchronization" "$klog" \
	> /dev/null; then
	echo "  Passed: timeout detected"
    else
	echo "  Failed: no timeout detected"
    fi
}

verify_exp_via_klog()
{
    [ $# -ge 2 ] || die "missing parameter for verrify_exp_via_klog"
    local klog="$1"
    shift
    if [ ! -f "$klog" ]; then
	echo "  Failed: No kernel log for checking MCE exp"
	return -1
    fi

    for exp in "$@"; do
	if grep "Machine check: " "$klog" | grep "$exp" > /dev/null; then
	    echo "  Passed: correct MCE exp"
	    return
	fi
    done
    echo "  Failed:  uncorrected MCE exp, expected: $exp"
}

get_panic_from_mcelog()
{
    [ $# -eq 1 ] || die "missing parameter for get_panic_from_mcelog"
    local mcelog="$1"
    local tmpf=$WDIR/get_panic_from_mcelog
    local addr
    if mcelog_filter $mcelog "#BANK 219#" | head -1 > $tmpf; then
	local F="$(sed '1,$s/#/\n/g' $tmpf | awk '/MISC / { print $2 }')"
	case "$F" in
	    0x1) echo "Fatal machine check" ;;
	    0x2) echo "Machine check from unknown source" ;;
	    0x3) echo "Uncorrected data corruption machine check" ;;
	    0x4) echo "Fatal machine check" ;;
	    *) echo unknown panic $F ;;
	esac
    fi
}

verify_panic_msg()
{
    [ $# -eq 2 ] || die "missing parameter for verify_panic_msg"
    local panic_msg="$1"
    local mce_panic="$2"

    if echo ": $panic_msg" | grep -e "$mce_panic" &> /dev/null; then
	echo "  Passed: correct panic"
    else
	echo "  Failed: uncorrect panic, expected: $mce_panic"
    fi
}

verify_timeout_via_mcelog()
{
    [ $# -eq 1 ] || die "missing parameter for verify_timeout"
    local mcelog="$1"

    if mcelog_filter $mcelog "#BANK 218#" &> /dev/null; then
	echo "  Passed: timeout detected"
    else
	echo "  Failed: no timeout detected"
    fi
}

set_tolerant()
{
    [ $# -eq 1 ] || die "missing parameter for set_tolerant"
    echo -n $1 > /sys/devices/system/machinecheck/machinecheck0/tolerant
}

get_tolerant()
{
    cat /sys/devices/system/machinecheck/machinecheck0/tolerant
}

check_debugfs()
{
	mount|grep /sys/kernel/debug > /dev/null 2>&1
	[ ! $? -eq 0 ] && mount -t debugfs none /sys/kernel/debug
	mount|grep /sys/kernel/debug > /dev/null 2>&1
	[ ! $? -eq 0 ] && die "Kernel without debugfs support ?"
}

# should be called after check_debugfs
check_mce()
{
    DEBUGFS=`mount | grep debugfs | cut -d ' ' -f3 | head -1`
    [ ! -d ${DEBUGFS}/mce ] && die "Kernel without CONFIG_X86_MCE_INJECT ?"
}

set_fake_panic()
{
    check_debugfs
    check_mce
    [ $# -eq 1 ] || die "missing parameter for set_fake_panic"
    echo -n $1 > /sys/kernel/debug/mce/fake_panic
}

set_panic_on_oops()
{
    [ $# -eq 1 ] || die "missing parameter for set_panic_on_oops"
    echo -n $1 > /proc/sys/kernel/panic_on_oops
}
