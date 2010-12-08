#!/bin/bash
#
# Software injection based test cases: test cases are triggered via
# the mce-inject tool.
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
    local over_exp="Overflowed uncorrected"
    local fatal_panic=": Fatal Machine check"
    local curr_cpu_panic=": Fatal machine check on current CPU"
    case "$bcase" in
	uc_over|uc_over_corrected)
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$fatal_panic"
	    soft_inject_verify_exp "$over_exp"
	    ;;
	uc_over_timeout)
	    soft_inject_verify_mcelog
	    verify_klog $klog
	    soft_inject_verify_panic "$curr_cpu_panic"
	    soft_inject_verify_exp "$over_exp"
	    soft_inject_verify_timeout
	    ;;
	*)
	    echo "!!! Unknown case: $this_case !!!"
    esac
}

soft_inject_main "$@"
