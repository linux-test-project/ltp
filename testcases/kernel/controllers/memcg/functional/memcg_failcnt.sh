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
TST_TEST_DATA="--mmap-anon --mmap-file --shm"

test()
{
	ROD echo $MEMORY_LIMIT \> memory.limit_in_bytes

	start_memcg_process $2 -s ${MEMORY_TO_ALLOCATE}
	ROD echo $MEMCG_PROCESS_PID \> tasks

	signal_memcg_process ${MEMORY_TO_ALLOCATE}
	signal_memcg_process ${MEMORY_TO_ALLOCATE}

	stop_memcg_process

	failcnt=$(cat memory.failcnt)
	if [ $failcnt -gt 0 ]; then
		tst_res TPASS "memory.failcnt is $failcnt, > 0 as expected"
	else
		tst_res TFAIL "memory.failcnt is $failcnt, <= 0 expected"
	fi
}

. memcg_lib.sh

MEMORY_LIMIT=$PAGESIZE
MEMORY_TO_ALLOCATE=$((MEMORY_LIMIT * 2))

tst_run
