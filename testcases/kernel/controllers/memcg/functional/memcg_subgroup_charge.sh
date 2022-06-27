#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Li Zefan <lizf@cn.fujitsu.com>
# Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>
# Added memcg enable/disable functinality: Rishikesh K Rajak <risrajak@linux.vnet.ibm.com

MEMCG_TESTFUNC=test
TST_CNT=3

# Test the memory charge won't move to subgroup
# $1 - memory.limit_in_bytes in sub group
test_subgroup()
{
	local limit_subgroup=$1

	if [ $limit_subgroup -ne 0 ]; then
		limit_subgroup=$(memcg_adjust_limit_for_kmem $limit_subgroup)
	fi

	ROD mkdir subgroup
	EXPECT_PASS echo $limit_subgroup \> subgroup/memory.limit_in_bytes

	start_memcg_process --mmap-anon -s $MEM_TO_ALLOC

	warmup
	if [ $? -ne 0 ]; then
		return
	fi

	echo $MEMCG_PROCESS_PID > tasks
	signal_memcg_process $MEM_TO_ALLOC
	check_mem_stat "rss" $MEM_TO_ALLOC

	cd subgroup
	echo $MEMCG_PROCESS_PID > tasks
	check_mem_stat "rss" 0

	# cleanup
	cd ..
	stop_memcg_process
	rmdir subgroup
}

test1()
{
	tst_res TINFO "Test that group and subgroup have no relationship"
	test_subgroup $((2 * MEM_TO_ALLOC))
}

test2()
{
	test_subgroup $MEM_TO_ALLOC
}

test3()
{
	test_subgroup 0
}

. memcg_lib.sh

# Allocate memory bigger than per-cpu kernel memory
MEM_TO_ALLOC=$((PAGESIZES * 2))

tst_run
