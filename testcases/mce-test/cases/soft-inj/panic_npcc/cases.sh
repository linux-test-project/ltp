#!/bin/bash
#
# Software injection based test cases: test cases are triggered via
# the mce-inject tool.
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is released under the GPLv2.
#

. $ROOT/lib/functions.sh
. $ROOT/lib/dirs.sh
. $ROOT/lib/mce.sh
. $ROOT/lib/soft-inject.sh

enumerate()
{
    soft_inject_enumerate
}

trigger()
{
    reset_severity_cov
    soft_inject_trigger
}

get_result()
{
    soft_inject_get_klog
    get_gcov arch/x86/kernel/cpu/mcheck/mce.c
    soft_inject_get_mcelog
    get_severity_cov
}

verify()
{
    local mce_panic
    local removes="TSC TIME PROCESSOR"
    local pcc_exp="Processor context corrupt"
    local knoripv_exp="In kernel and no restart IP"
    local no_mcip_exp="MCIP not set in MCA handler"
    local no_eripv_exp="Neither restart nor error IP"
    local over_exp="Overflowed uncorrected"
    local fatal_panic=": Fatal Machine check"
    local curr_cpu_panic=": Fatal machine check on current CPU"
    local unknown_src_panic=": Machine check from unknown source"
    case "$bcase" in
	fatal_severity)
	    removes="$removes RIP"
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$fatal_panic"
	    soft_inject_verify_exp "$pcc_exp"
	    ;;
	uncorrected)
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$fatal_panic"
	    soft_inject_verify_exp "$knoripv_exp"
	    ;;
	uncorrected_timeout*)
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$curr_cpu_panic"
	    soft_inject_verify_timeout
	    soft_inject_verify_exp "$knoripv_exp"
	    ;;
	uc_no_mcip)
	    removes="$removes RIP"
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$fatal_panic"
	    soft_inject_verify_exp "$no_mcip_exp"
	    ;;
	uc_no_mcip_timeout)
	    removes="$removes RIP"
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$curr_cpu_panic"
	    soft_inject_verify_exp "$no_mcip_exp"
	    soft_inject_verify_timeout
	    ;;
	uc_no_eripv)
	    removes="$removes RIP"
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$fatal_panic"
	    soft_inject_verify_exp "$no_eripv_exp"
	    ;;
	uc_no_eripv_timeout)
	    removes="$removes RIP"
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$curr_cpu_panic"
	    soft_inject_verify_exp "$no_eripv_exp"
	    soft_inject_verify_timeout
	    ;;
	unknown)
	    verify_klog $klog
	    soft_inject_verify_panic "$unknown_src_panic"
	    ;;
	*)
	    echo "!!! Unknown case: $this_case !!!"
    esac
}

soft_inject_main "$@"
