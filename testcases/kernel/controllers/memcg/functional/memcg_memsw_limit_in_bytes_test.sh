#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2012 FUJITSU LIMITED
# Copyright (c) 2014-2016 Linux Test Project
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Peng Haitao <penght@cn.fujitsu.com>

MEMCG_TESTFUNC=test
TST_CNT=12


test1()
{
	test_proc_kill $PAGESIZE "--mmap-lock1" $((PAGESIZE*2)) 1
}

test2()
{
	test_proc_kill $PAGESIZE "--mmap-lock2" $((PAGESIZE*2)) 1
}

test3()
{
	test_proc_kill 0 "--mmap-anon" $PAGESIZE 1
}

test4()
{
	test_proc_kill 0 "--mmap-file" $PAGESIZE 1
}

test5()
{
	test_proc_kill 0 "--shm -k 21" $PAGESIZE 1
}

test6()
{
	test_limit_in_bytes $((PAGESIZE - 1)) 1
}

test7()
{
	test_limit_in_bytes $((PAGESIZE + 1)) 1
}

test8()
{
	test_limit_in_bytes 1 1
}

test9()
{
	memcg_require_memsw

	ROD echo 10M \> memory.limit_in_bytes

	if tst_kvcmp -lt "2.6.31"; then
		EXPECT_FAIL echo -1 \> memory.memsw.limit_in_bytes
	else
		EXPECT_PASS echo -1 \> memory.memsw.limit_in_bytes
	fi
}

test10()
{
	memcg_require_memsw

	ROD echo 10M \> memory.limit_in_bytes
	EXPECT_FAIL echo 1.0 \> memory.memsw.limit_in_bytes
}

test11()
{
	memcg_require_memsw

	ROD echo 10M \> memory.limit_in_bytes
	EXPECT_FAIL echo 1xx \> memory.memsw.limit_in_bytes
}

test12()
{
	memcg_require_memsw

	ROD echo 10M \> memory.limit_in_bytes
	EXPECT_FAIL echo xx \> memory.memsw.limit_in_bytes
}

. memcg_lib.sh
tst_run
