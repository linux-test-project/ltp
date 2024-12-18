// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2013-2017  Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2014-2023
 */
/*\
 * [Description]
 *
 * Out Of Memory (OOM) test for MEMCG and CPUSET
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
	tst_res(TINFO, "OOM on CPUSET & MEMCG...");
	testoom(0, 0, ENOMEM, 1);

	/*
	 * Under NUMA system, the migration of cpuset's memory
	 * is in charge of cpuset.memory_migrate, we can write
	 * 1 to cpuset.memory_migrate to enable the migration.
	 */
	if (is_numa(NULL, NH_MEMS, 2) &&
	    SAFE_CG_HAS(tst_cg, "cpuset.memory_migrate")) {
		SAFE_CG_PRINT(tst_cg, "cpuset.memory_migrate", "1");
		tst_res(TINFO, "OOM on CPUSET & MEMCG with "
				"cpuset.memory_migrate=1");
		testoom(0, 0, ENOMEM, 1);
	}

	if (SAFE_CG_HAS(tst_cg, "memory.swap.max")) {
		tst_res(TINFO, "OOM on CPUSET & MEMCG with "
				"special memswap limitation:");
		if (!TST_CG_VER_IS_V1(tst_cg, "memory"))
			SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%lu", MB);
		else
			SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%lu", TESTMEM + MB);

		testoom(0, 1, ENOMEM, 1);

		tst_res(TINFO, "OOM on CPUSET & MEMCG with "
				"disabled memswap limitation:");
		if (TST_CG_VER_IS_V1(tst_cg, "memory"))
			SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%lu", ~0UL);
		else
			SAFE_CG_PRINT(tst_cg, "memory.swap.max", "max");
		testoom(0, 0, ENOMEM, 1);
	}
}

void setup(void)
{
	int ret, memnode;

	if (!is_numa(NULL, NH_MEMS, 1))
		tst_brk(TCONF, "requires NUMA with at least 1 node");

	/*
	 * Some nodes do not contain memory, so use
	 * get_allowed_nodes(NH_MEMS) to get a memory
	 * node. This operation also applies to Non-NUMA
	 * systems.
	 */
	ret = get_allowed_nodes(NH_MEMS, 1, &memnode);
	if (ret < 0)
		tst_brk(TBROK, "Failed to get a memory node "
			      "using get_allowed_nodes()");

	write_node_cpusets(tst_cg, memnode);
	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());
	SAFE_CG_PRINTF(tst_cg, "memory.max", "%lu", TESTMEM);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.timeout = TST_UNLIMITED_TIMEOUT,
	.setup = setup,
	.test_all = verify_oom,
	.needs_cgroup_ctrls = (const char *const []){
		"memory", "cpuset", NULL
	},
	.skip_in_compat = 1,
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/vm/overcommit_memory", "1", TST_SR_TBROK},
		{}
	},
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
