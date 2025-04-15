#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2009 FUJITSU LIMITED
# Author: Li Zefan <lizf@cn.fujitsu.com>

TST_TESTFUNC=test
TST_SETUP=do_setup
TST_CLEANUP=do_cleanup
TST_CNT=8
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="awk dmesg find mountpoint rmdir"

do_setup()
{
	mkdir cgroup/

	if [ ! -f /proc/cgroups ]; then
		tst_brk TCONF ignored "Kernel does not support for control groups; skipping testcases";
	fi

	dmesg -c > /dev/null
	NR_BUG=`dmesg | grep -c "kernel BUG"`
	NR_NULL=`dmesg | grep -c "kernel NULL pointer dereference"`
	NR_WARNING=`dmesg | grep -c "^WARNING"`
	NR_LOCKDEP=`dmesg | grep -c "possible recursive locking detected"`
}

do_cleanup()
{
	if mountpoint -q cgroup/; then
		find cgroup/ -maxdepth 1 -depth -exec rmdir {} +
		umount cgroup/
		rmdir cgroup
	fi
}

check_kernel_bug()
{
	local id="$1"
	local ok_msg="no kernel bug was found"
	local new_bug=`dmesg | grep -c "kernel BUG"`
	local new_null=`dmesg | grep -c "kernel NULL pointer dereference"`
	local new_warning=`dmesg | grep -c "^WARNING"`
	local new_lockdep=`dmesg | grep -c "possible recursive locking detected"`

	[ "$id" ] && ok_msg="$ok_msg for test $i"

	# no kernel bug is detected
	if [ $new_bug -eq $NR_BUG -a $new_warning -eq $NR_WARNING -a \
	     $new_null -eq $NR_NULL -a $new_lockdep -eq $NR_LOCKDEP ]; then
		tst_res TPASS $ok_msg
		return 0
	fi

	# some kernel bug is detected
	if [ $new_bug -gt $NR_BUG ]; then
		tst_res TFAIL "kernel BUG was detected!"
	fi
	if [ $new_warning -gt $NR_WARNING ]; then
		tst_res TFAIL "kernel WARNING was detected!"
	fi
	if [ $new_null -gt $NR_NULL ]; then
		tst_res TFAIL "kernel NULL pointer dereference!"
	fi
	if [ $new_lockdep -gt $NR_LOCKDEP ]; then
		tst_res TFAIL "kernel lockdep warning was detected!"
	fi

	NR_BUG=$new_bug
	NR_NULL=$new_null
	NR_WARNING=$new_warning
	NR_LOCKDEP=$new_lockdep

	tst_res TWARN "BUG FOUND!"
	dmesg
	return 1
}

#---------------------------------------------------------------------------
# Bug:    There was a race when keeping forking processes and at the same
#         time cat /cgroup/tasks (should be the very first time to read
#         /cgroup/tasks, otherwise this bug won't be triggered)
# Kernel: 2.6.24, 2.6.25-rcX
# Links:  http://lkml.org/lkml/2007/10/17/224
#         http://lkml.org/lkml/2008/3/5/332
#         http://lkml.org/lkml/2008/4/16/493
# Fix:    commit 0e04388f0189fa1f6812a8e1cb6172136eada87e
#---------------------------------------------------------------------------
test1()
{
	cgroup_regression_fork_processes &
	sleep 1

	mount -t cgroup -o none,name=foo cgroup cgroup/
	if [ $? -ne 0 ]; then
		tst_res TFAIL "failed to mount cgroup filesystem"
		kill -TERM $!
		return
	fi
	cat cgroup/tasks > /dev/null

	kill -TERM $!
	wait $! 2>/dev/null
	umount cgroup/
	check_kernel_bug
}

#---------------------------------------------------------------------------
# Bug:    a cgroup's notify_on_release flag did not inherit from its parent.
# Kernel: 2.6.24-rcX
# Links:  http://lkml.org/lkml/2008/2/25/12
# Fix:    commit bc231d2a048010d5e0b49ac7fddbfa822fc41109
#---------------------------------------------------------------------------
test2()
{
	local val1
	local val2

	mount -t cgroup -o none,name=foo cgroup cgroup/
	if [ $? -ne 0 ]; then
		tst_res TFAIL "Failed to mount cgroup filesystem"
		return
	fi

	echo 0 > cgroup/notify_on_release
	mkdir cgroup/0
	val1=`cat cgroup/0/notify_on_release`

	echo 1 > cgroup/notify_on_release
	mkdir cgroup/1
	val2=`cat cgroup/1/notify_on_release`

	if [ $val1 -ne 0 -o $val2 -ne 1 ]; then
		tst_res TFAIL "wrong notify_on_release value"
	else
		tst_res TPASS "notify_on_release is inherited"
	fi

	rmdir cgroup/0 cgroup/1
	tst_umount $PWD/cgroup
}

#---------------------------------------------------------------------------
# Bug:    Accessing NULL cgrp->dentry when reading /proc/sched_debug
# Kernel: 2.6.26-2.6.28
# Links:  http://lkml.org/lkml/2008/10/30/44
#         http://lkml.org/lkml/2008/12/12/107
#         http://lkml.org/lkml/2008/12/16/481
# Fix:    commit a47295e6bc42ad35f9c15ac66f598aa24debd4e2
#---------------------------------------------------------------------------
test3()
{
	local cpu_subsys_path

	if [ ! -e /proc/sched_debug ]; then
		tst_res TCONF "CONFIG_SCHED_DEBUG is not enabled"
		return
	fi

	if ! grep -q -w "cpu" /proc/cgroups; then
		tst_res TCONF "CONFIG_CGROUP_SCHED is not enabled"
		return
	fi

	cgroup_require "cpu"
	cpu_subsys_path=$(cgroup_get_mountpoint "cpu")

	cgroup_regression_3_1.sh $cpu_subsys_path &
	pid1=$!
	cgroup_regression_3_2.sh &
	pid2=$!

	sleep 30
	kill -USR1 $pid1 $pid2
	wait $pid1 2>/dev/null
	wait $pid2 2>/dev/null

	rmdir $cpu_subsys_path/0 2> /dev/null
	cgroup_cleanup
	check_kernel_bug
}

#---------------------------------------------------------------------------
# Bug:    cgroup hierarchy lock's lockdep subclass may overflow
# Kernel: 2.6.29-rcX
# Link:   http://lkml.org/lkml/2009/2/4/67
# Fix:
#---------------------------------------------------------------------------
test4()
{
	local lines

	if [ ! -e /proc/lockdep ]; then
		tst_res TCONF "CONFIG_LOCKDEP is not enabled"
		return
	fi

	# MAX_LOCKDEP_SUBCLASSES is 8, so number of subsys should be > 8
	lines=`cat /proc/cgroups | wc -l`
	if [ $lines -le 9 ]; then
		tst_res TCONF "require more than 8 cgroup subsystems"
		return
	fi

	mount -t cgroup -o none,name=foo cgroup cgroup/
	mkdir cgroup/0
	rmdir cgroup/0
	tst_umount $PWD/cgroup

	if dmesg | grep -q "MAX_LOCKDEP_SUBCLASSES too low"; then
		tst_res TFAIL "lockdep BUG was found"
		return
	fi

	tst_res TPASS "no lockdep BUG was found"
}

#---------------------------------------------------------------------------
# Bug:    When running 2 concurrent mount/umount threads, kernel WARNING
#         may be triggered, but it's VFS' issue but not cgroup.
# Kernel: 2.6.24 - 2.6.29-rcX
# Links:  http://lkml.org/lkml/2009/1/4/354
# Fix:    commit 1a88b5364b535edaa321d70a566e358390ff0872
#---------------------------------------------------------------------------
test5()
{
	cgroup_regression_5_1.sh &
	local pid1=$!
	cgroup_regression_5_2.sh &
	local pid2=$!

	sleep 30
	kill -USR1 $pid1 $pid2
	wait $pid1 2>/dev/null
	wait $pid2 2>/dev/null

	mount -t cgroup none cgroup 2> /dev/null
	mkdir cgroup/0
	rmdir cgroup/0
	tst_umount $PWD/cgroup
	check_kernel_bug
}

#---------------------------------------------------------------------------
# Bug:    When running 2 concurrent mount/umount threads, lockdep warning
#         may be triggered, it's a false positive, and it's VFS' issue but
#         not cgroup.
# Kernel: 2.6.24 - 2.6.29-rcX
# Links:  http://lkml.org/lkml/2009/1/4/352
# Fix:    commit ada723dcd681e2dffd7d73345cc8fda0eb0df9bd
#---------------------------------------------------------------------------
test6()
{
	cgroup_regression_6_1.sh &
	local pid1=$!
	cgroup_regression_6_2.sh &
	local pid2=$!

	sleep 30
	kill -USR1 $pid1 $pid2
	wait $pid1 2>/dev/null
	wait $pid2 2>/dev/null

	umount cgroup/ 2> /dev/null
	check_kernel_bug
}

#---------------------------------------------------------------------------
# Bug:    There was a bug when remount cgroup fs with some dead subdirs in
#         it (rmdir()ed but still has some refcnts on it). It caused memory
#         leak, and may cause oops when cat /proc/sched_debug.
# Kernel: 2.6.24 - 2.6.27, 2.6.28-rcX
# Links:  http://lkml.org/lkml/2008/12/10/369
# Fix:    commit 307257cf475aac25db30b669987f13d90c934e3a
#---------------------------------------------------------------------------
test_7_1()
{
	local subsys=$1
	local subsys_path
	# we should be careful to select a $subsys_path which is related to
	# cgroup only: if cgroup debugging is enabled a 'debug' $subsys
	# could be passed here as params and this will lead to ambiguity and
	# errors when grepping simply for 'debug' in /proc/mounts since we'll
	# find also /sys/kernel/debug. Helper takes care of this.

	cgroup_require "$subsys"
	subsys_path=$(cgroup_get_mountpoint "$subsys")

	mkdir $subsys_path/0
	sleep 100 < $subsys_path/0 &	# add refcnt to this dir
	rmdir $subsys_path/0

	# remount with new subsystems added
	# since 2.6.28, this remount will fail

	if [ "$subsys_path" = "cgroup" ]; then
		mount -t cgroup -o remount xxx cgroup/ 2> /dev/null
		kill -TERM $!
		wait $! 2>/dev/null
		umount cgroup/
	fi

	cgroup_cleanup
}

test_7_2()
{
	local subsys=$1

	mount -t cgroup -o none,name=foo cgroup cgroup/
	if [ $? -ne 0 ]; then
		tst_res TFAIL "failed to mount cgroup"
		return
	fi

	mkdir cgroup/0
	sleep 100 < cgroup/0 &	# add refcnt to this dir
	rmdir cgroup/0

	# remount with some subsystems removed
	# since 2.6.28, this remount will fail
	mount -t cgroup -o remount,$subsys xxx cgroup/ 2> /dev/null
	kill -TERM $!
	wait $! 2>/dev/null
	umount cgroup/

	grep -q -w "cpu" /proc/cgroups
	if [ $? -ne 0 -o ! -e /proc/sched_debug ]; then
		tst_res TCONF "skip rest of testing due possible oops triggered by reading /proc/sched_debug"
		return
	fi

	tmp=0
	while [ $tmp -lt 50 ]; do
		echo 3 > /proc/sys/vm/drop_caches
		cat /proc/sched_debug > /dev/null
		tmp=$((tmp+1))
	done
}

test7()
{
	local lines=`cat /proc/cgroups | wc -l`
	local subsys
	local i=1

	if [ $lines -le 2 ]; then
		tst_res TCONF "require at least 2 cgroup subsystems"
		slt_result $SLT_Untested
		return
	fi

	subsys=$(awk 'END{ print $1 }' /proc/cgroups)

	# remount to add new subsystems to the hierarchy
	while [ $i -le 2 ]; do
		test_7_$i $subsys || return
		check_kernel_bug $i || return
		i=$((i+1))
	done
}

#---------------------------------------------------------------------------
# Bug:    oops when get cgroupstat of a cgroup control file
# Kernel: 2.6.24 - 2.6.27, 2.6.28-rcX
# Links:  http://lkml.org/lkml/2008/11/19/53
# Fix:    commit 33d283bef23132c48195eafc21449f8ba88fce6b
#---------------------------------------------------------------------------
test8()
{
	mount -t cgroup -o none,name=foo cgroup cgroup/
	if [ $? -ne 0 ]; then
		tst_res TFAIL "failed to mount cgroup filesystem"
		return
	fi

	if cgroup_regression_getdelays -C cgroup/tasks > /dev/null 2>&1; then
		tst_res TFAIL "should have failed to get cgroupstat of tasks file"
	fi

	umount cgroup/
	check_kernel_bug
}

. cgroup_lib.sh
tst_run
