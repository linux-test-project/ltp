#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2012 FUJITSU LIMITED
# Copyright (c) 2014-2016 Linux Test Project
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Peng Haitao <penght@cn.fujitsu.com>

MEMCG_TESTFUNC=test
TST_CNT=2

. memcg_lib.sh

MEM_TO_ALLOC=$((PAGESIZE * 1024))
MEM_LIMIT=$((MEM_TO_ALLOC * 2))

test1()
{
	tst_res TINFO "Test memory.usage_in_bytes"
	test_mem_stat "--mmap-anon" $MEM_TO_ALLOC $MEM_TO_ALLOC \
		"memory.usage_in_bytes" $MEM_TO_ALLOC \
		$((MEM_TO_ALLOC + PAGESIZE * 32)) false
}

test2()
{
	tst_res TINFO "Test memory.memsw.usage_in_bytes"
	memcg_require_memsw

	echo $MEM_LIMIT > memory.limit_in_bytes
	echo $MEM_LIMIT > memory.memsw.limit_in_bytes
	test_mem_stat "--mmap-anon" $MEM_TO_ALLOC $MEM_TO_ALLOC \
		"memory.memsw.usage_in_bytes" $MEM_TO_ALLOC \
		$((MEM_TO_ALLOC + PAGESIZE * 32)) false
}

tst_run
