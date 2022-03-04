#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Li Zefan <lizf@cn.fujitsu.com>
# Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>
# Added memcg enable/disable functionality: Rishikesh K Rajak <risrajak@linux.vnet.ibm.com>

MEMCG_TESTFUNC=test
TST_CNT=6

. memcg_lib.sh

# Test memory.force_empty
test1()
{
	start_memcg_process --mmap-anon -s $PAGESIZE
	echo $MEMCG_PROCESS_PID > tasks
	signal_memcg_process $PAGESIZE
	echo $MEMCG_PROCESS_PID > ../tasks

	# This expects that there is swap configured
	EXPECT_PASS echo 1 \> memory.force_empty

	stop_memcg_process $pid
}

test2()
{
	EXPECT_PASS echo 0 \> memory.force_empty
}

test3()
{
	EXPECT_PASS echo 1.0 \> memory.force_empty
}

test4()
{
	EXPECT_PASS echo 1xx \> memory.force_empty
}

test5()
{
	EXPECT_PASS echo xx \> memory.force_empty
}

test6()
{
	# writing to non-empty top mem cgroup's force_empty
	# should return failure
	EXPECT_FAIL echo 1 \> "$mount_point/memory.force_empty"
}

tst_run
