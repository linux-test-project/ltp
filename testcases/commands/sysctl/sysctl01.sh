#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
#
# This is a regression test for invalid value of sysctl_sched_time_avg.
# System will hang if user set sysctl_sched_time_avg to 0 on buggy kernel.
#
# The kernel bug has been fixed in kernel:
# '5ccba44ba118("sched/sysctl: Check user input value of sysctl_sched_time_avg")'

TST_TESTFUNC=sysctl_test
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="sysctl"

sysctl_test()
{
	# With commit d00535d, sched_time_avg was renamed as sched_time_avg_ms
	local dir="/proc/sys/kernel/"
	[ -e "$dir""sched_time_avg_ms" ] && local name="sched_time_avg_ms"
	[ -e "$dir""sched_time_avg" ] && local name="sched_time_avg"
	[ -z "$name" ] && tst_brk TCONF \
		"sched_time_avg(_ms) was not supported"

	local orig_value=$(cat "$dir""$name")

	sysctl -w "kernel.""$name"=0 >/dev/null 2>&1

	local test_value=$(cat "$dir""$name")

	if [ ${test_value} -eq ${orig_value} ]; then
		tst_res TPASS "Setting $name failed"
	else
		tst_res TFAIL "Setting $name succeeded"
		sysctl -w "kernel.""$name"=${orig_value} >/dev/null 2>&1
	fi
}

. tst_test.sh
tst_run
