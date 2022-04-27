#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
# Copyright (c) 2018 Linux Test Project
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Li Zefan <lizf@cn.fujitsu.com>
# Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>
# Added memcg enable/disable functinality: Rishikesh K Rajak <risrajak@linux.vnet.ibm.com

MEMCG_TESTFUNC=test
MEMCG_SHMMAX=1
TST_CNT=10


# Test the management and counting of memory
test1()
{
	test_mem_stat "--mmap-anon" $PAGESIZES $PAGESIZES "rss" $PAGESIZES $PAGESIZES false
}

test2()
{
	test_mem_stat "--mmap-file" $PAGESIZE $PAGESIZE "rss" 0 0 false
}

test3()
{
	test_mem_stat "--shm -k 3" $PAGESIZE $PAGESIZE "rss" 0 0 false
}

test4()
{
	test_mem_stat "--mmap-anon --mmap-file --shm" \
		$PAGESIZES $((PAGESIZES * 3)) "rss" $PAGESIZES $PAGESIZES false
}

test5()
{
	test_mem_stat "--mmap-lock1" $PAGESIZES $PAGESIZES "rss" $PAGESIZES $PAGESIZES false
}

test6()
{
	test_mem_stat "--mmap-anon" $PAGESIZES $PAGESIZES "rss" $PAGESIZES $PAGESIZES true
}

test7()
{
	test_mem_stat "--mmap-file" $PAGESIZE $PAGESIZE "rss" 0 0 true
}

test8()
{
	test_mem_stat "--shm -k 8" $PAGESIZE $PAGESIZE "rss" 0 0 true
}

test9()
{
	test_mem_stat "--mmap-anon --mmap-file --shm" \
		$PAGESIZES $((PAGESIZES * 3)) "rss" $PAGESIZES $PAGESIZES true
}

test10()
{
	test_mem_stat "--mmap-lock1" $PAGESIZES $PAGESIZES "rss" $PAGESIZES $PAGESIZES true
}

. memcg_lib.sh
tst_run
