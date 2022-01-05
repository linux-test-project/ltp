/*
 * Out Of Memory (OOM) for MEMCG and CPUSET
 *
 * The program is designed to cope with unpredictable like amount and
 * system physical memory, swap size and other VMM technology like KSM,
 * memcg, memory hotplug and so on which may affect the OOM
 * behaviours. It simply increase the memory consumption 3G each time
 * until all the available memory is consumed and OOM is triggered.
 *
 * Copyright (C) 2013-2017  Red Hat, Inc.
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

static const struct tst_cgroup_group *cg;

static void verify_oom(void)
{
#ifdef TST_ABI32
	tst_brk(TCONF, "test is not designed for 32-bit system.");
#endif

	tst_res(TINFO, "OOM on CPUSET & MEMCG...");
	testoom(0, 0, ENOMEM, 1);

	/*
	 * Under NUMA system, the migration of cpuset's memory
	 * is in charge of cpuset.memory_migrate, we can write
	 * 1 to cpuset.memory_migrate to enable the migration.
	 */
	if (is_numa(NULL, NH_MEMS, 2) &&
	    SAFE_CGROUP_HAS(cg, "cpuset.memory_migrate")) {
		SAFE_CGROUP_PRINT(cg, "cpuset.memory_migrate", "1");
		tst_res(TINFO, "OOM on CPUSET & MEMCG with "
				"cpuset.memory_migrate=1");
		testoom(0, 0, ENOMEM, 1);
	}

	if (SAFE_CGROUP_HAS(cg, "memory.swap.max")) {
		tst_res(TINFO, "OOM on CPUSET & MEMCG with "
				"special memswap limitation:");
		if (!TST_CGROUP_VER_IS_V1(cg, "memory"))
			SAFE_CGROUP_PRINTF(cg, "memory.swap.max", "%lu", MB);
		else
			SAFE_CGROUP_PRINTF(cg, "memory.swap.max", "%lu", TESTMEM + MB);

		testoom(0, 1, ENOMEM, 1);

		tst_res(TINFO, "OOM on CPUSET & MEMCG with "
				"disabled memswap limitation:");
		if (TST_CGROUP_VER_IS_V1(cg, "memory"))
			SAFE_CGROUP_PRINTF(cg, "memory.swap.max", "%lu", ~0UL);
		else
			SAFE_CGROUP_PRINT(cg, "memory.swap.max", "max");
		testoom(0, 0, ENOMEM, 1);
	}
}

void setup(void)
{
	int ret, memnode;

	if (!is_numa(NULL, NH_MEMS, 1))
		tst_brk(TCONF, "requires NUMA with at least 1 node");

	overcommit = get_sys_tune("overcommit_memory");
	set_sys_tune("overcommit_memory", 1, 1);

	tst_cgroup_require("memory", NULL);
	tst_cgroup_require("cpuset", NULL);
	cg = tst_cgroup_get_test_group();

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

	write_cpusets(cg, memnode);
	SAFE_CGROUP_PRINTF(cg, "cgroup.procs", "%d", getpid());
	SAFE_CGROUP_PRINTF(cg, "memory.max", "%lu", TESTMEM);
}

void cleanup(void)
{
	if (overcommit != -1)
		set_sys_tune("overcommit_memory", overcommit, 0);
	tst_cgroup_cleanup();
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.timeout = -1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_oom,
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
