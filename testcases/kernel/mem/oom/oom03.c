/*
 * Out Of Memory (OOM) for Memory Resource Controller
 *
 * The program is designed to cope with unpredictable like amount and
 * system physical memory, swap size and other VMM technology like KSM,
 * memcg, memory hotplug and so on which may affect the OOM
 * behaviours. It simply increase the memory consumption 3G each time
 * until all the available memory is consumed and OOM is triggered.
 *
 * Copyright (C) 2010-2017  Red Hat, Inc.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif

#include "lapi/abisize.h"
#include "numa_helper.h"
#include "mem.h"

#ifdef HAVE_NUMA_V2

static void verify_oom(void)
{
#ifdef TST_ABI32
	tst_brk(TCONF, "test is not designed for 32-bit system.");
#endif
	testoom(0, 0, ENOMEM, 1);

	if (SAFE_CG_HAS(tst_cg, "memory.swap.max")) {
		tst_res(TINFO, "OOM on MEMCG with special memswap limitation:");
		/*
		 * Cgroup v2 tracks memory and swap in separate, which splits
		 * memory and swap counter. So with swappiness enable (default
		 * value is 60 on RHEL), it likely has part of memory swapping
		 * out during the allocating.
		 *
		 * To get more opportunities to reach the swap limitation,
		 * let's scale down the value of 'memory.swap.max' to only
		 * 1MB for CGroup v2.
		 */
		if (!TST_CG_VER_IS_V1(tst_cg, "memory"))
			SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%lu", MB);
		else
			SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%lu", TESTMEM + MB);

		testoom(0, 1, ENOMEM, 1);

		if (TST_CG_VER_IS_V1(tst_cg, "memory"))
			SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%lu", ~0UL);
		else
			SAFE_CG_PRINT(tst_cg, "memory.swap.max", "max");
	}

	/* OOM for MEMCG with mempolicy */
	if (is_numa(NULL, NH_MEMS, 2)) {
		tst_res(TINFO, "OOM on MEMCG & mempolicy...");
		testoom(MPOL_BIND, 0, ENOMEM, 1);
		testoom(MPOL_INTERLEAVE, 0, ENOMEM, 1);
		testoom(MPOL_PREFERRED, 0, ENOMEM, 1);
	}
}

static void setup(void)
{
	overcommit = get_sys_tune("overcommit_memory");
	set_sys_tune("overcommit_memory", 1, 1);

	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());
	SAFE_CG_PRINTF(tst_cg, "memory.max", "%lu", TESTMEM);
}

static void cleanup(void)
{
	if (overcommit != -1)
		set_sys_tune("overcommit_memory", overcommit, 0);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.max_runtime = TST_UNLIMITED_RUNTIME,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_oom,
	.needs_cgroup_ctrls = (const char *const []){ "memory", NULL },
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
