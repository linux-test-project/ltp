#!/bin/bash
#
# APEI injection based test cases : memory patrol scrub cases: test cases
# are triggered via the apei-inject, and they will not trigger kernel panic.
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#           Zheng Jiajia <jiajia.zheng@intel.com>
#
# This file is released under the GPLv2.
#

. $ROOT/lib/functions.sh
. $ROOT/lib/dirs.sh
. $ROOT/lib/mce.sh
. $ROOT/lib/apei-inject.sh

enumerate()
{
    apei_inject_enumerate
}

trigger()
{
    case "$bcase" in
        mem_uncorrected)
            start_tracing "do_machine_check"
            ;;
        *)
            echo '!!! Unknown case: $this_case !!!'
    esac
    apei_inject_trigger
    stop_tracing
}

get_result()
{
    apei_inject_get_klog
    get_gcov drivers/acpi/apei/einj.c

    case "$bcase" in
	mem_uncorrected)
	    get_mcelog_from_dev $mcelog_result
	    ;;
	*)
	    echo '!!! Unknown case: $this_case !!!'
    esac
}

verify()
{
    case "$bcase" in
	mem_uncorrected)
	    apei_inject_verify_mcelog
	    verify_klog $klog
            apei_inject_verify_trace "do_machine_check"
	    ;;
	*)
	    echo "!!! Unknown case: $this_case !!!"
    esac
}

apei_inject_main "$@"
