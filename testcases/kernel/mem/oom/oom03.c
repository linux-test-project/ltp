// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2010-2017  Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2011-2023
 */
/*\
 * [Description]
 *
 * Out Of Memory (OOM) test for Memory Resource Controller
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

#include "numa_helper.h"
#include "mem.h"

#ifdef HAVE_NUMA_V2

static void verify_oom(void)
{
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
	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());
	SAFE_CG_PRINTF(tst_cg, "memory.max", "%lu", TESTMEM);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.max_runtime = TST_UNLIMITED_RUNTIME,
	.setup = setup,
	.test_all = verify_oom,
	.needs_cgroup_ctrls = (const char *const []){ "memory", NULL },
	.skip_in_compat = 1,
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/vm/overcommit_memory", "1", TST_SR_TBROK},
		{}
	},
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
