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

. memcg_lib.sh

# Test the memory charge won't move to subgroup
# $1 - memory.limit_in_bytes in parent group
# $2 - memory.limit_in_bytes in sub group
test_subgroup()
{
	mkdir subgroup
	echo $1 > memory.limit_in_bytes
	echo $2 > subgroup/memory.limit_in_bytes

	start_memcg_process --mmap-anon -s $PAGESIZES

	warmup
	if [ $? -ne 0 ]; then
		return
	fi

	echo $MEMCG_PROCESS_PID > tasks
	signal_memcg_process $PAGESIZES
	check_mem_stat "rss" $PAGESIZES

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
	test_subgroup $PAGESIZES $((2 * PAGESIZES))
}

test2()
{
	test_subgroup $PAGESIZES $PAGESIZES
}

test3()
{
	test_subgroup $PAGESIZES 0
}

tst_run
