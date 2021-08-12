#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2012 FUJITSU LIMITED
# Copyright (c) 2014-2016 Linux Test Project
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Peng Haitao <penght@cn.fujitsu.com>

MEMCG_TESTFUNC=test
TST_CNT=4

. memcg_lib.sh

MEM_TO_ALLOC=$((PAGESIZE * 1024))
MEM_EXPECTED_UPPER=$((MEM_TO_ALLOC + MEM_USAGE_RANGE))
MEM_LIMIT=$((MEM_TO_ALLOC * 2))

# Run test cases which checks memory.[memsw.]max_usage_in_bytes after make
# some memory allocation
test_max_usage_in_bytes()
{
	local item="memory.max_usage_in_bytes"
	[ $1 -eq 1 ] && item="memory.memsw.max_usage_in_bytes"
	local check_after_reset=$2
	local exp_stat_size_low=$MEM_TO_ALLOC
	local exp_stat_size_up=$MEM_EXPECTED_UPPER
	local kmem_stat_name="${item##*.}"

	start_memcg_process --mmap-anon -s $MEM_TO_ALLOC

	warmup
	if [ $? -ne 0 ]; then
		return
	fi

	ROD echo $MEMCG_PROCESS_PID \> tasks
	signal_memcg_process $MEM_TO_ALLOC
	signal_memcg_process $MEM_TO_ALLOC

	if [ "$kmem_stat_name" = "max_usage_in_bytes" ] ||
	   [ "$kmem_stat_name" = "usage_in_bytes" ]; then
		local kmem=$(cat "memory.kmem.${kmem_stat_name}")
		if [ $? -eq 0 ]; then
			exp_stat_size_low=$((exp_stat_size_low + kmem))
			exp_stat_size_up=$((exp_stat_size_up + kmem))
		fi
	fi

	check_mem_stat $item $exp_stat_size_low $exp_stat_size_up

	if [ $check_after_reset -eq 1 ]; then
		echo 0 > $item
		check_mem_stat $item 0 $PAGESIZES
	fi

	stop_memcg_process
}

test1()
{
	tst_res TINFO "Test memory.max_usage_in_bytes"
	test_max_usage_in_bytes 0 0
}

test2()
{
	tst_res TINFO "Test memory.memsw.max_usage_in_bytes"
	memcg_require_memsw

	echo $MEM_LIMIT > memory.limit_in_bytes
	echo $MEM_LIMIT > memory.memsw.limit_in_bytes
	test_max_usage_in_bytes 1 0
}

test3()
{
	tst_res TINFO "Test reset memory.max_usage_in_bytes"
	test_max_usage_in_bytes 0 1
}

test4()
{
	tst_res TINFO "Test reset memory.memsw.max_usage_in_bytes"
	memcg_require_memsw

	echo $MEM_LIMIT > memory.limit_in_bytes
	echo $MEM_LIMIT > memory.memsw.limit_in_bytes
	test_max_usage_in_bytes 1 1
}

tst_run
