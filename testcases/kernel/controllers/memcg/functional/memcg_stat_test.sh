#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2012 FUJITSU LIMITED
# Copyright (c) 2014-2018 Linux Test Project
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Peng Haitao <penght@cn.fujitsu.com>

MEMCG_TESTFUNC=test
TST_CNT=8


test1()
{
	tst_res TINFO "Test cache"
	test_mem_stat "--shm -k 3" $PAGESIZES $PAGESIZES "cache" $PAGESIZES $PAGESIZES false
}

test2()
{
	tst_res TINFO "Test mapped_file"
	test_mem_stat "--mmap-file" $PAGESIZES $PAGESIZES \
		"mapped_file" $PAGESIZES $PAGESIZES false
}

test3()
{
	tst_res TINFO "Test unevictable with MAP_LOCKED"
	test_mem_stat "--mmap-lock1" $PAGESIZES $PAGESIZES \
		"unevictable" $PAGESIZES $PAGESIZES false
}

test4()
{
	tst_res TINFO "Test unevictable with mlock"
	test_mem_stat "--mmap-lock2" $PAGESIZES $PAGESIZES \
		"unevictable" $PAGESIZES $PAGESIZES false
}

test5()
{
	tst_res TINFO "Test hierarchical_memory_limit with enabling hierarchical accounting"
	echo 1 > memory.use_hierarchy

	local limit=$(memcg_adjust_limit_for_kmem $PAGESIZES)

	ROD mkdir subgroup
	EXPECT_PASS echo $limit \> memory.limit_in_bytes
	EXPECT_PASS echo $((limit + PAGESIZES * 2)) \> subgroup/memory.limit_in_bytes

	cd subgroup
	check_mem_stat "hierarchical_memory_limit" $limit

	cd ..
	rmdir subgroup
}

test6()
{
	tst_res TINFO "Test hierarchical_memory_limit with disabling hierarchical accounting"
	memcg_require_hierarchy_disabled

	ROD echo 0 \> memory.use_hierarchy

	ROD mkdir subgroup
	EXPECT_PASS echo $PAGESIZES \> memory.limit_in_bytes
	EXPECT_PASS echo $((PAGESIZES * 2)) \> subgroup/memory.limit_in_bytes

	cd subgroup
	check_mem_stat "hierarchical_memory_limit" $((PAGESIZES * 2))

	cd ..
	rmdir subgroup
}

test7()
{
	tst_res TINFO "Test hierarchical_memsw_limit with enabling hierarchical accounting"
	memcg_require_memsw

	ROD echo 1 \> memory.use_hierarchy

	ROD mkdir subgroup
	EXPECT_PASS echo $PAGESIZES \> memory.limit_in_bytes
	EXPECT_PASS echo $PAGESIZES \> memory.memsw.limit_in_bytes
	EXPECT_PASS echo $((PAGESIZES * 2)) \> subgroup/memory.limit_in_bytes
	EXPECT_PASS echo $((PAGESIZES * 2)) \> subgroup/memory.memsw.limit_in_bytes

	cd subgroup
	check_mem_stat "hierarchical_memsw_limit" $PAGESIZES

	cd ..
	rmdir subgroup
}

test8()
{
	tst_res TINFO "Test hierarchical_memsw_limit with disabling hierarchical accounting"
	memcg_require_memsw
	memcg_require_hierarchy_disabled

	ROD echo 0 \> memory.use_hierarchy

	ROD mkdir subgroup
	EXPECT_PASS echo $PAGESIZES \> memory.limit_in_bytes
	EXPECT_PASS echo $PAGESIZES \> memory.memsw.limit_in_bytes
	EXPECT_PASS echo $((PAGESIZES * 2)) \> subgroup/memory.limit_in_bytes
	EXPECT_PASS echo $((PAGESIZES * 2)) \> subgroup/memory.memsw.limit_in_bytes

	cd subgroup
	check_mem_stat "hierarchical_memsw_limit" $((PAGESIZES * 2))

	cd ..
	rmdir subgroup
}

. memcg_lib.sh
tst_run
