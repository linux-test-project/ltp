#!/bin/bash
#
# Simple test driver: run test cases one by one, assuming test case
# will not trigger panic or reboot.
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is released under the GPLv2.
#

sd=$(dirname "$0")
export ROOT=`(cd $sd/../..; pwd)`

export driver=simple

. $ROOT/lib/functions.sh
setup_path
. $ROOT/lib/dirs.sh
. $ROOT/lib/mce.sh

tmp_klog=$WDIR/simple_klog_tmp

chk_err()
{
    if [ \( -n "$DEBUG_MCE_TEST" \) -a \( -s $err_log \) ]; then
	cat $err_log
    fi
}

klog_begin()
{
    dmesg > $tmp_klog.1
}

klog_end()
{
    dmesg | sed -e '1d' > $tmp_klog.2
    diff $tmp_klog.1 $tmp_klog.2 | grep '^> ' | sed 's/> \(.*\)/\1/' > $klog
}

trigger()
{
    reset_gcov

    $CDIR/$case_sh trigger
}

get_result()
{
    $CDIR/$case_sh get_result
}

test_all()
{
    if [ -n "$GCOV" ]; then
	if chk_gcov; then
	    export GCOV=copy
	    export KSRC_DIR
	else
	    echo "gcov is not supported by kernel or there is no " \
		"gcov utility installed"
	    echo "disabling gcov support."
	    unset GCOV
	fi
    fi

    #if mce_inject is a module, it is ensured to have been loaded
    modinfo mce_inject > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        lsmod | grep mce_inject > /dev/null 2>&1
        [ $? -eq 0 ] || modprobe mce_inject
        [ $? -eq 0 ] || die "module mce_inject isn't supported ?"
    fi

    for case_sh in $CASES; do
	for this_case in $($CDIR/$case_sh enumerate); do
	    set_fake_panic 1

	    export this_case
	    mkdir -p $RDIR/$this_case
	    rm -rf $RDIR/$this_case/*
	    echo -e "\n$this_case:" | tee -a $RDIR/result
	    klog=$RDIR/$this_case/klog

	    mkdir -p $WDIR/$this_case
	    rm -rf $WDIR/$this_case/*
	    local err_log=$WDIR/$this_case/err_log

	    klog_begin
	    trigger 2>$err_log | tee -a $RDIR/result
	    chk_err
	    random_sleep
	    klog_end
	    get_result 2>$err_log | tee -a $RDIR/result
	    chk_err
	    $CDIR/$case_sh verify 2>$err_log | tee -a $RDIR/result
	    chk_err

	    set_fake_panic 0
	done
    done
}

if [ $# -lt 1 ]; then
    die "Usage: $0 <config>"
fi

conf=$(basename "$1")

. $CONF_DIR/$conf

driver_prepare
set_panic_on_oops 0

if [ -n "$START_BACKGROUND" ]; then
    eval $START_BACKGROUND
else
    start_background
fi

[ -d $RDIR ] && mv $RDIR --backup=numbered -T $RDIR.old
[ -d $WDIR ] && mv $WDIR --backup=numbered -T $WDIR.old

test_all

if [ -n "$STOP_BACKGROUND" ]; then
    eval $STOP_BACKGROUND
else
    stop_background
fi

