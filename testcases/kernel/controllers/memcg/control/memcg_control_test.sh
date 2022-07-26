#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2010 Mohamed Naufal Basheer
# Author: Mohamed Naufal Basheer

TST_TESTFUNC=test
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_CNT=1
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1

PAGE_SIZE=$(tst_getconf PAGESIZE)

TOT_MEM_LIMIT=$PAGE_SIZE
ACTIVE_MEM_LIMIT=$PAGE_SIZE
PROC_MEM=$((PAGE_SIZE * 2))

STATUS_PIPE="status_pipe"

# Check if the test process is killed on crossing boundary
test_proc_kill()
{
	mem_process -m $PROC_MEM &
	sleep 1
	ROD echo $! \> "$test_dir/$task_list"

	#Instruct the test process to start acquiring memory
	echo m > $STATUS_PIPE
	sleep 5

	#Check if killed
	ps -p $! > /dev/null 2> /dev/null
	if [ $? -eq 0 ]; then
		echo m > $STATUS_PIPE
		echo x > $STATUS_PIPE
	else
		: $((KILLED_CNT += 1))
	fi
}

# Validate the memory usage limit imposed by the hierarchically topmost group
test1()
{
	cd $TST_TMPDIR

	tst_res TINFO "Test #1: Checking if the memory usage limit imposed by the topmost group is enforced"

	ROD echo "$ACTIVE_MEM_LIMIT" \> "$test_dir/$memory_limit"
	ROD echo "$TOT_MEM_LIMIT" \> "$test_dir/$memsw_memory_limit"

	KILLED_CNT=0
	test_proc_kill

	if [ $PROC_MEM -gt $TOT_MEM_LIMIT ] && [ $KILLED_CNT -eq 0 ]; then
		tst_res TFAIL "Test #1: failed"
	else
		tst_res TPASS "Test #1: passed"
	fi
}

setup()
{
	cgroup_require "memory"
	cgroup_version=$(cgroup_get_version "memory")
	test_dir=$(cgroup_get_test_path "memory")
	task_list=$(cgroup_get_task_list "memory")

	if [ "$cgroup_version" = "2" ]; then
		memory_limit="memory.max"
		memsw_memory_limit="memory.swap.max"
	else
		memory_limit="memory.limit_in_bytes"
		memsw_memory_limit="memory.memsw.limit_in_bytes"
	fi

	tst_res TINFO "Test starts with cgroup version $cgroup_version"
}

cleanup()
{
	cgroup_cleanup
}

. cgroup_lib.sh
tst_run
