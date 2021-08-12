#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2012 FUJITSU LIMITED
# Copyright (c) 2014-2016 Linux Test Project
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Peng Haitao <penght@cn.fujitsu.com>

MEMCG_TESTFUNC=test
TST_CNT=3

. memcg_lib.sh

test1()
{
	tst_res TINFO "test if one of the ancestors goes over its limit, the proces will be killed"

	local limit=$(memcg_adjust_limit_for_kmem $PAGESIZE)

	ROD echo 1 \> memory.use_hierarchy
	ROD echo $limit \> memory.limit_in_bytes

	ROD mkdir subgroup
	cd subgroup
	test_proc_kill $((limit + PAGESIZE * 3)) "--mmap-lock1" $((limit + PAGESIZE * 2)) 0

	cd ..
	rmdir subgroup
}

test2()
{
	tst_res TINFO "test Enabling will fail if the cgroup already has other cgroups"

	memcg_require_hierarchy_disabled

	ROD mkdir subgroup
	EXPECT_FAIL echo 1 \> memory.use_hierarchy

	rmdir subgroup
}

test3()
{
	tst_res TINFO "test disabling will fail if the parent cgroup has enabled hierarchy"

	memcg_require_hierarchy_disabled

	ROD echo 1 > memory.use_hierarchy
	mkdir subgroup
	EXPECT_FAIL echo 0 \> subgroup/memory.use_hierarchy

	rmdir subgroup
}

tst_run
