#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Author: Li Zefan <lizf@cn.fujitsu.com>
# Added memcg enable/disable functionality: Rishikesh K Rajak <risrajak@linux.vnet.ibm.com>

TST_ID="memcg_regression_test"
TST_CLEANUP=cleanup
TST_SETUP=setup
TST_TESTFUNC=test_
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="killall kill"
TST_CNT=4

#buffer can rotate and number of found bugs can actually go down
#so clear the buffer to avoid this
dmesg -c > /dev/null
nr_bug=0
nr_null=0
nr_warning=0
nr_lockdep=0

# check_kernel_bug - check if some kind of kernel bug happened
check_kernel_bug()
{
	new_bug=`dmesg | grep -c "kernel BUG"`
	new_null=`dmesg | grep -c "kernel NULL pointer dereference"`
	new_warning=`dmesg | grep -c "^WARNING"`
	new_lockdep=`dmesg | grep -c "possible recursive locking detected"`

	# no kernel bug is detected
	if [ $new_bug -eq $nr_bug -a $new_warning -eq $nr_warning -a \
	     $new_null -eq $nr_null -a $new_lockdep -eq $nr_lockdep ]; then
		return 1
	fi

	# some kernel bug is detected
	if [ $new_bug -gt $nr_bug ]; then
		tst_res TFAIL "kernel BUG was detected!"
	fi
	if [ $new_warning -gt $nr_warning ]; then
		tst_res TFAIL "kernel WARNING was detected!"
	fi
	if [ $new_null -gt $nr_null ]; then
		tst_res TWARN "kernel NULL pointer dereference!"
	fi
	if [ $new_lockdep -gt $nr_lockdep ]; then
		tst_res TWARN "kernel lockdep warning was detected!"
	fi

	nr_bug=$new_bug
	nr_null=$new_null
	nr_warning=$new_warning
	nr_lockdep=$new_lockdep

	echo "check_kernel_bug found something!"
	dmesg
	return 0
}

setup()
{
	if tst_kvcmp -lt "2.6.30"; then
		tst_brk TBROK "Test should be run with kernel 2.6.30 or newer"
	fi

	cgroup_require "memory"
	cgroup_version=$(cgroup_get_version "memory")
	mount_point=$(cgroup_get_mountpoint "memory")
	test_dir=$(cgroup_get_test_path "memory")
	task_list=$(cgroup_get_task_list "memory")
	if [ "$cgroup_version" = "2" ]; then
		memory_limit="memory.max"
	else
		memory_limit="memory.limit_in_bytes"
	fi

	[ "$cgroup_version" = "2" ] && ROD echo "+memory" \> "$test_dir/cgroup.subtree_control"

	tst_res TINFO "test starts with cgroup version $cgroup_version"
}

cleanup()
{
	cleanup_testpath "$test_dir/0"
	cgroup_cleanup
}

create_testpath()
{
	local path="$1"
	[ ! -e "$path" ] && ROD mkdir "$path"
}

cleanup_testpath()
{
	local path="$1"
	[ -e "$path" ] && ROD rmdir "$path"
}

#---------------------------------------------------------------------------
# Bug:    The bug was, while forking mass processes, trigger memcgroup OOM,
#         then NULL pointer dereference may be hit.
# Kernel: 2.6.25-rcX
# Links:  http://lkml.org/lkml/2008/4/14/38
# Fix:    commit e115f2d89253490fb2dbf304b627f8d908df26f1
#---------------------------------------------------------------------------
test_1()
{
	local test_path
	test_path="$test_dir/0"

	create_testpath "$test_path"
	ROD echo 0 \> "$test_path/$memory_limit"

	./memcg_test_1 "$test_path/$task_list"

	cleanup_testpath "$test_path"

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_res TPASS "no kernel bug was found"
	fi
}

#---------------------------------------------------------------------------
# Bug:    Shrink memory might never return, unless send signal to stop it.
# Kernel: 2.6.29
# Links:  http://marc.info/?t=123199973900003&r=1&w=2
#         http://lkml.org/lkml/2009/2/3/72
# Fix:    81d39c20f5ee2437d71709beb82597e2a38efbbc
#---------------------------------------------------------------------------
test_2()
{
	local test_path

	# for cgroup2 writing to memory.max first checks the new limit against the
	# current usage and will start killing processes if oom, therefore we do not
	# expect EBUSY to be returned by the shrink operation.
	if [ "$cgroup_version" = "2" ]; then
		tst_res TCONF "Cgroup v2 found, skipping test"
		return
	fi

	test_path="$test_dir/0"

	./memcg_test_2 &
	pid1=$!
	sleep 1

	create_testpath "$test_path"
	ROD echo $pid1 \> "$test_path"/tasks

	# let pid1 'test_2' allocate memory
	/bin/kill -SIGUSR1 $pid1
	sleep 1

	# shrink memory
	echo 1 > "$test_path"/memory.limit_in_bytes 2>&1 &
	pid2=$!

	# check if 'echo' will exit and exit with failure
	for tmp in $(seq 0 4); do
		sleep 1
		ps -p $! > /dev/null
		if [ $? -ne 0 ]; then
			wait $pid2
			if [ $? -eq 0 ]; then
				tst_res TFAIL "echo should return failure"
				kill -9 $pid1 $pid2 > /dev/null 2>&1
				wait $pid1 $pid2
				cleanup_testpath "$test_path"
				return
			fi
			break
		fi
	done

	if [ $tmp -eq 5 ]; then
		tst_res TFAIL "'echo' doesn't exit!"
	else
		tst_res TPASS "EBUSY was returned as expected"
	fi

	kill -9 $pid1 $pid2 > /dev/null 2>&1
	wait $pid1 $pid2 > /dev/null 2>&1
	cleanup_testpath "$test_path"
}

#---------------------------------------------------------------------------
# Bug:    crash when rmdir a cgroup on IA64
# Kernel: 2.6.29-rcX
# Links:  http://marc.info/?t=123235660300001&r=1&w=2
# Fix:    commit 299b4eaa302138426d5a9ecd954de1f565d76c94
#---------------------------------------------------------------------------
test_3()
{
	local test_path
	test_path="$test_dir/0"
	create_testpath "$test_path"

	for pid in $(cat "$mount_point/$task_list"); do
		echo $pid > "$test_path/$task_list" 2> /dev/null
	done

	for pid in $(cat "$test_path/$task_list"); do
		echo $pid > "$mount_point/$task_list" 2> /dev/null
	done
	cleanup_testpath "$test_path"

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_res TPASS "no kernel bug was found"
	fi
}

#---------------------------------------------------------------------------
# Bug:    the memcg's refcnt handling at swapoff was wrong, causing crash
# Kernel: 2.6.29-rcX
# Links:  http://marc.info/?t=123208656300004&r=1&w=2
# Fix:    commit 85d9fc89fb0f0703df6444f260187c088a8d59ff
#---------------------------------------------------------------------------
test_4()
{
	local test_path
	test_path="$test_dir/0"
	create_testpath "$test_path"

	./memcg_test_4.sh "$cgroup_version" "$mount_point" "$test_path"

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_res TPASS "no kernel bug was found"
	fi

	# test_4.sh might be killed by oom, so do clean up here
	killall -9 memcg_test_4 2> /dev/null
	killall -9 memcg_test_4.sh 2> /dev/null

	# if test_4.sh gets killed, it won't clean cgroup it created
	cleanup_testpath "$test_path"

	swapon -a
}

. cgroup_lib.sh
tst_run
