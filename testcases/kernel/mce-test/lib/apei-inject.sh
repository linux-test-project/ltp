#
# APEI library: APEI specific functions
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#           Zheng Jiajia <jiajia.zheng@intel.com>
# This file is released under the GPLv2.
#

. $ROOT/lib/soft-inject.sh

if [ -n "$this_case" ]; then
    bcase=$(basename $this_case)
fi
mcelog_result=$RDIR/$this_case/mcelog
klog=$RDIR/$this_case/klog

apei_mce_reformat()
{
    local inf="$1"
    local outf="$2"
    local tmpf=$WDIR/mce_reformat_for_cmp
    sed "s/0x//g" $inf | grep -v 'STATUS 0x0' | \
        grep -v 'STATUS 0x800000000000000' | sort > "$tmpf"
    awk '/^STATUS/ {print $2}' $tmpf | cut -b 1-3 > $outf
    awk '/MCGSTATUS/ {if ($4=="") print $2; else print $4;}' $tmpf >> $outf
}

apei_mce_cmp()
{
    [ $# -eq 2 ] || die "missing parameter for mce_cmp"
    local m1="$1"
    local m2="$2"
    local tmpf1=$WDIR/mce_cmp_1
    local tmpf2=$WDIR/mce_cmp_2

    apei_mce_reformat "$m1" $tmpf1 
    apei_mce_reformat "$m2" $tmpf2 
    diff $tmpf1 $tmpf2 > /dev/null
}

apei_inject_verify_mcelog()
{
    if [ -f $RDIR/$this_case/mcelog ]; then
        mcelog_refer=$SDIR/refer/$bcase
        mce-inject --dump $mcelog_refer > $RDIR/$this_case/mcelog_refer
        if apei_mce_cmp $RDIR/$this_case/mcelog $RDIR/$this_case/mcelog_refer; then 
            echo "  Passed: MCE log is ok"
        else
            echo "  Failed: MCE log is different from input"
        fi
    else
        echo "  Failed: no MCE log result"
    fi
}


apei_inject_get_klog()
{
    soft_inject_get_klog
}

apei_inject_get_mcelog()
{
    soft_inject_get_mcelog
}

# verify return value
apei_inject_verify_return_val()
{
    soft_inject_verify_return_val
}

apei_inject_verify_kill()
{
    soft_inject_verify_kill
}

apei_inject_enumerate()
{
    soft_inject_enumerate
}

apei_inject_trigger()
{
    check_debugfs
    #APEI_IF should be defined after debugfs is mounted
    APEI_IF=`mount | grep debugfs | cut -d ' ' -f3`/apei/einj

    #if einj is a module, it is ensured to have been loaded
    modinfo einj > /dev/null 2>&1
    if [ $? -eq 0 ]; then
	[ -d $APEI_IF ] || modprobe einj
        [ $? -eq 0 ] || die "module einj isn't supported ?"
    fi

    mcelog &> /dev/null
    TYPE=`awk '/^TYPE/{print $2}' $SDIR/data/$bcase`
    echo $TYPE > $APEI_IF/error_type
    killall simple_process
    $TDIR/simple_process/simple_process > /dev/null &

    page-types -p `pidof simple_process` -LN -b ano > $RDIR/$this_case/page

    ADDR=`awk '$2 != "offset" {print "0x"$2"000"}' $RDIR/$this_case/page | sed -n -e '1p'`
    echo $ADDR > $APEI_IF/param1

    echo "1" > $APEI_IF/error_inject

    ret=$?
    echo $ret > $RDIR/$this_case/return
    sleep 1
}

start_tracing()
{
    [ $# -eq 1 ] || die "missing parameter for get_panic_from_mcelog: please set filter for ftrace"
    [ -d /sys/kernel/debug/tracing ] || die "no tracing"
    echo "function_graph" > /sys/kernel/debug/tracing/current_tracer
    echo $1 > /sys/kernel/debug/tracing/set_ftrace_filter
    echo "1" > /sys/kernel/debug/tracing/tracing_enabled
}

stop_tracing()
{
    [ -d /sys/kernel/debug/tracing ] || die "no tracing"
    echo "0" > /sys/kernel/debug/tracing/tracing_enabled
    cp /sys/kernel/debug/tracing/trace $RDIR/$this_case/
    echo "nop" > /sys/kernel/debug/tracing/current_tracer
}

apei_inject_verify_trace()
{
    [ $# -eq 1 ] || die "missing parameter for apei_inject_verify_trace"
    if grep "$1" $RDIR/$this_case/trace; then
       echo "Passed: trace is correct"
    else
       echo "Failed: Nothing is traced"
    fi
}

apei_inject_verify_panic()
{
    local mce_panic="$1"
    verify_panic_via_klog $klog "$mce_panic"
}

apei_inject_verify_exp()
{
    verify_exp_via_klog $klog "$@"
}

apei_inject_verify_fail()
{
    verify_fail_via_klog $klog "$@"
}

apei_inject_main()
{
    op="$1"
    shift

    case "$op" in
	enumerate)
	    enumerate
	    ;;
	trigger)
	    trigger "$@"
	    ;;
	get_result)
	    get_result
	    ;;
	verify)
	    verify
	    ;;
	*)
	    die "Usage: $0 enumerate|trigger|get_result|verify"
    esac
    exit 0
}
