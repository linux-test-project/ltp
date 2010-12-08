#!/bin/bash
#
# Software injection based test cases: test cases are triggered via
# mce-inject tool.
#
# Copyright (C) 2009, Intel Corp.
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
    local removes="TSC TIME PROCESSOR"
    local fatal_panic=": Fatal Machine check"
    local curr_cpu_panic=": Fatal machine check on current CPU"
    local unknown_src_panic=": Machine check from unknown source"
    local no_eripv_exp="Neither restart nor error IP"
    case "$bcase" in
	s0_ar1)
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$fatal_panic"
	    soft_inject_verify_exp "Illegal combination (UCNA with AR=1)"
	    ;;
	srar_over)
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$fatal_panic"
	    soft_inject_verify_exp "Action required with lost events"
	    ;;
	srar_unkown)
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$fatal_panic"
	    soft_inject_verify_exp "Action required; unknown MCACOD"
	    ;;
        srar_no_en)
            soft_inject_verify_mcelog
            verify_klog $klog
            soft_inject_verify_panic "Machine check from unknown source"
            ;;
	srao_mem_scrub_noripv|srao_ewb_noripv)
            soft_inject_verify_mcelog
            verify_klog $klog
            soft_inject_verify_panic "$fatal_panic"
            soft_inject_verify_exp "$no_eripv_exp"
            ;;
	*)
	    echo "!!! Unknown case: $this_case !!!"
    esac
}

soft_inject_main "$@"
