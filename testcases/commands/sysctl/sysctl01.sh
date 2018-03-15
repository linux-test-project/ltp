#!/bin/sh

# Copyright (c) 2018 FUJITSU LIMITED. All rights reserved.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#
# Description:
# This is a regression test for invalid value of sysctl_sched_time_avg.
# System will hang if user set sysctl_sched_time_avg to 0 on buggy kernel.
#
# The kernel bug has been fixed in kernel:
# '5ccba44ba118("sched/sysctl: Check user input value of sysctl_sched_time_avg")'

TST_TESTFUNC=sysctl_test
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="sysctl"

. tst_test.sh

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

tst_run
