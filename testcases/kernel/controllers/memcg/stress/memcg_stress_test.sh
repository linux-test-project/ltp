#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Copyright (c) 2018-2019 ARM Ltd. All Rights Reserved.
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
#
# Author: Li Zefan <lizf@cn.fujitsu.com>
# Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>
# Added memcg enable/disable functionality: Rishikesh K Rajak <risrajak@linux.vnet.ibm.com>

TST_TESTFUNC=test
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_CNT=2
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="mount umount cat kill mkdir rmdir grep awk cut"

# Each test case runs for 900 secs when everything fine
# therefore the default 5 mins timeout is not enough.
TST_TIMEOUT=2100

. cgroup_lib.sh

setup()
{
	cgroup_require "memory"
	cgroup_version=$(cgroup_get_version "memory")
	test_path=$(cgroup_get_test_path "memory")
	task_list=$(cgroup_get_task_list "memory")
	if [ "$cgroup_version" = "2" ]; then
		ROD echo "+memory" > "$test_path/cgroup.subtree_control"
	fi

	tst_res TINFO "test starts with cgroup version $cgroup_version"

	echo 3 > /proc/sys/vm/drop_caches
	sleep 2
	local mem_free=`cat /proc/meminfo | grep MemFree | awk '{ print $2 }'`
	local swap_free=`cat /proc/meminfo | grep SwapFree | awk '{ print $2 }'`
	local pgsize=`tst_getconf PAGESIZE`

	MEM=$(( $mem_free + $swap_free / 2 ))
	MEM=$(( $MEM / 1024 ))
	RUN_TIME=$(( 15 * 60 ))
	[ "$pgsize" = "4096" ] && THREAD_SPARE_MB=1 || THREAD_SPARE_MB=8

	tst_res TINFO "Calculated available memory $MEM MB"
}

cleanup()
{
	cgroup_cleanup
}

# $1 Number of cgroups
# $2 Allocated MB memory in one process
# $3 The interval to touch memory in a process
# $4 Test duration (sec)
run_stress()
{
	local cgroups="$1"
	local mem_size="$2"
	local interval="$3"
	local timeout="$4"
	local i pid pids

	tst_res TINFO "Testing $cgroups cgroups, using $mem_size MB, interval $interval"

	tst_res TINFO "Starting cgroups"
	for i in $(seq 0 $(($cgroups-1))); do
		ROD mkdir "$test_path/$i"
		memcg_process_stress $mem_size $interval &
		ROD echo $! > "$test_path/$i/$task_list"
		pids="$pids $!"
	done

	for pid in $pids; do
		kill -USR1 $pid 2> /dev/null
	done

	tst_res TINFO "Testing cgroups for ${timeout}s"
	sleep $timeout

	tst_res TINFO "Killing groups"
	i=0
	for pid in $pids; do
		kill -KILL $pid 2> /dev/null
		wait $pid 2> /dev/null
		ROD rmdir "$test_path/$i"
		i=$((i+1))
	done

	tst_res TPASS "Test passed"
}

test1()
{
	run_stress 150 $(( ($MEM - 150 * $THREAD_SPARE_MB) / 150 )) 5 $RUN_TIME
}

test2()
{
	run_stress 1 $(( $MEM - $THREAD_SPARE_MB)) 5 $RUN_TIME
}

tst_run
