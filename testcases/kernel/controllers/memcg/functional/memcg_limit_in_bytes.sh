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
MEMCG_SHMMAX=1
TST_CNT=15

TST_CLEANUP=cleanup

cleanup()
{
	memcg_cleanup
	swapon -a
}

test1()
{
	tst_res TINFO "Test mmap(locked) + alloc_mem > limit_in_bytes"
	test_proc_kill $PAGESIZE "--mmap-lock1" $((PAGESIZE * 2)) 0
}

test2()
{
	test_proc_kill $PAGESIZE "--mmap-lock2" $((PAGESIZE * 2)) 0
}

test3()
{
	tst_res TINFO "Test swapoff + alloc_mem > limit_in_bytes"
	swapoff -a
	test_proc_kill $PAGESIZE "--mmap-anon" $((PAGESIZE * 2)) 0
	swapon -a
}

test4()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--mmap-file" $((PAGESIZE * 2)) 0
	swapon -a
}

test5()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--shm -k 18" $((PAGESIZE * 2)) 0
	swapon -a
}

test6()
{
	tst_res TINFO "Test limit_in_bytes == 0"
	test_proc_kill 0 "--mmap-anon" $PAGESIZE 0
}

test7()
{
	test_proc_kill 0 "--mmap-file" $PAGESIZE 0
}

test8()
{
	test_proc_kill 0 "--shm -k 21" $PAGESIZE 0
}

test9()
{
	tst_res TINFO "Test limit_in_bytes will be aligned to PAGESIZE"
	test_limit_in_bytes $((PAGESIZE - 1)) 0
}

test10()
{
	test_limit_in_bytes $((PAGESIZE + 1)) 0
}

test11()
{
	test_limit_in_bytes 1 0
}

test12()
{
	tst_res TINFO "Test invalid memory.limit_in_bytes"
	if tst_kvcmp -lt "2.6.31"; then
		EXPECT_FAIL echo -1 \> memory.limit_in_bytes
	else
		EXPECT_PASS echo -1 \> memory.limit_in_bytes
	fi
}

test13()
{
	EXPECT_FAIL echo 1.0 \> memory.limit_in_bytes
}

test14()
{
	EXPECT_FAIL echo 1xx \> memory.limit_in_bytes
}

test15()
{
	EXPECT_FAIL echo xx \> memory.limit_in_bytes
}

. memcg_lib.sh
tst_run
