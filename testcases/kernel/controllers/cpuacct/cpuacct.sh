#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 SUSE
# Author: Cedric Hnyda <chnyda@suse.com>
#
# Usage
# ./cpuacct.sh nbsubgroup nbprocess
# nbsubgroup: number of subgroup to create
# nbprocess: number of process to attach to each subgroup
#
# Description
# 1) Find if cpuacct is mounted, if not mounted, cpuacct will be mounted
# 2) Check that sum ltp_test/subgroup*/cpuacct.usage = ltp_test/cpuacct.usage

TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_POS_ARGS=2
TST_USAGE=usage
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="awk"

. tst_test.sh

mounted=1
max=$1
nbprocess=$2

usage()
{
	cat << EOF
usage: $0 nsubgroup nprocess

nsubgroup - number of subgroups to create
nprocess  - number of processes to attach to each subgroup

OPTIONS
EOF
}

check_free_memory()
{
	local memneeded
	local memfree=`awk '/MemAvailable/ {print $2}' /proc/meminfo`

	if [ $? -ne 0 ]; then
		local memcached

		memfree=`awk '/MemFree/ {print $2}' /proc/meminfo`
		test $? -eq 0 || return 0

		memcached=`awk '/MemCached/ {print $2}' /proc/meminfo`
		test $? -eq 0 || return 0

		memfree=$((memfree + memcached))
	fi

	# On x86_64, each 100 of processes were using ~16 MB of memory,
	# so try to estimate the needed free memory based on this.
	memneeded=$((max * nbprocess * 16384 / 100))

	if [ $memfree -lt $memneeded ]; then
		tst_brk TCONF "not enough of free memory on this system (approximate need $memneeded kB, free $memfree kB)"
	fi
	tst_res TINFO "memory requirements fulfilled (approximate need $memneeded kB, free $memfree kB)"

	return 0
}

check_limits()
{
	local tasksneeded=$((max * nbprocess))
	local tasksmax=$(tst_get_free_pids)
	test $? -eq 0 || return 0

	if [ $tasksmax -le $tasksneeded ]; then
		tst_brk TCONF "limit of tasks is too low (approximate need $tasksneeded, limit $tasksmax)"
	fi
	tst_res TINFO "task limit fulfilled (approximate need $tasksneeded, limit $tasksmax)"

	return 0
}

setup()
{
	if ! grep -q -w cpuacct /proc/cgroups; then
		tst_brk TCONF "cpuacct not supported on this system"
	fi

	check_limits
	# Don't bother with memory limit checks on smaller tests
	if [ $max -ge 100 ] && [ $nbprocess -ge 100 ]; then
		check_free_memory
	fi

	mount_point=`grep -w cpuacct /proc/mounts | cut -f 2 | cut -d " " -f2`
	tst_res TINFO "cpuacct: $mount_point"
	if [ "$mount_point" = "" ]; then
		mounted=0
		mount_point=/dev/cgroup
	fi

	testpath=$mount_point/ltp_$TST_ID

	if [ "$mounted" -eq "0" ]; then
		ROD mkdir -p $mount_point
		ROD mount -t cgroup -o cpuacct none $mount_point
	fi

	ROD mkdir $testpath

	# create subgroups
	for i in `seq 1 $max`; do
		ROD mkdir $testpath/subgroup_$i
	done

}

cleanup()
{

	if [ "$testpath" ]; then
		tst_res TINFO "removing created directories"
		rm -rf $testpath
	fi

	if [ "$mounted" -ne 1 ]; then
		tst_res TINFO "Umounting cpuacct"
		umount $mount_point
		rmdir $mount_point
	fi
}

do_test()
{
	tst_res TINFO "Creating $max subgroups each with $nbprocess processes"

	# create and attach process to subgroups
	for i in `seq 1 $max`; do
		for j in `seq 1 $nbprocess`; do
			cpuacct_task $testpath/subgroup_$i/tasks &
			echo $! >> task_pids
		done
	done

	for pid in $(cat task_pids); do wait $pid; done
	rm -f task_pids

	acc=0
	fails=0
	for i in `seq 1 $max`; do
		tmp=`cat $testpath/subgroup_$i/cpuacct.usage`
		if [ "$tmp" -eq "0" ]; then
			fails=$((fails + 1))
		fi
		acc=$((acc + tmp))
	done

	## check that cpuacct.usage != 0 for every subgroup
	if [ "$fails" -gt "0" ]; then
		tst_res TFAIL "cpuacct.usage is not equal to 0 for $fails subgroups"
	else
		tst_res TPASS "cpuacct.usage is not equal to 0 for every subgroup"
	fi

	## check that ltp_subgroup/cpuacct.usage == sum ltp_subgroup/subgroup*/cpuacct.usage
	ref=`cat $testpath/cpuacct.usage`
	if [ "$ref" -ne "$acc" ]; then
		tst_res TFAIL "cpuacct.usage $ref not equal to subgroup*/cpuacct.usage $acc"
	else
		tst_res TPASS "cpuacct.usage equal to subgroup*/cpuacct.usage"
	fi
}

tst_run
