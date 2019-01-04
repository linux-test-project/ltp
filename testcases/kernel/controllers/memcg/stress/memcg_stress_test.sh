#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Copyright (c) 2018-2019 ARM Ltd. All Rights Reserved.
#
# Author: Li Zefan <lizf@cn.fujitsu.com>
# Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>
# Added memcg enable/disable functionality: Rishikesh K Rajak <risrajak@linux.vnet.ibm.com

TST_TESTFUNC=test
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_CNT=2
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="mount umount cat kill mkdir rmdir grep awk cut"

# Each test case runs for 900 secs when everything fine
# therefore the default 5 mins timeout is not enough.
LTP_TIMEOUT_MUL=7

. cgroup_lib.sh

setup()
{
	if ! is_cgroup_subsystem_available_and_enabled "memory"; then
		tst_brk TCONF "Either kernel does not support Memory Resource Controller or feature not enabled"
	fi

	echo 3 > /proc/sys/vm/drop_caches
	sleep 2
	local mem_free=`cat /proc/meminfo | grep MemFree | awk '{ print $2 }'`
	local swap_free=`cat /proc/meminfo | grep SwapFree | awk '{ print $2 }'`

	MEM=$(( $mem_free + $swap_free / 2 ))
	MEM=$(( $MEM / 1024 ))
	RUN_TIME=$(( 15 * 60 ))

	tst_res TINFO "Calculated available memory $MEM MB"
}

cleanup()
{
	if [ -e /dev/memcg ]; then
		umount /dev/memcg 2> /dev/null
		rmdir /dev/memcg 2> /dev/null
	fi
}

do_mount()
{
	cleanup

	mkdir /dev/memcg 2> /dev/null
	mount -t cgroup -omemory memcg /dev/memcg
}

# $1 - Number of cgroups
# $2 - Allocated how much memory in one process? in MB
# $3 - The interval to touch memory in a process
# $4 - How long does this test run ? in second
run_stress()
{
	local i

	do_mount

	for i in $(seq 0 $(($1-1))); do
		mkdir /dev/memcg/$i 2> /dev/null
		memcg_process_stress $2 $3 &
		eval pid$i=$!

		eval echo \$pid$i > /dev/memcg/$i/tasks
	done

	for i in $(seq 0 $(($1-1))); do
		eval kill -USR1 \$pid$i 2> /dev/null
	done

	sleep $4

	for i in $(seq 0 $(($1-1))); do
		eval kill -KILL \$pid$i 2> /dev/null
		eval wait \$pid$i

		rmdir /dev/memcg/$i 2> /dev/null
	done

	cleanup
}

test1()
{
	tst_res TINFO "testcase 1 started...it will run for $RUN_TIME secs"

	run_stress 150 $(( ($MEM - 150) / 150 )) 5 $RUN_TIME

	tst_res TPASS "stress test 1 passed"
}

test2()
{
	tst_res TINFO "testcase 2 started...it will run for $RUN_TIME secs"

	run_stress 1 $MEM 5 $RUN_TIME

	tst_res TPASS "stress test 2 passed"
}

tst_run
