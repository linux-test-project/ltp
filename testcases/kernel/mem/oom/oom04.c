/*
 * Out Of Memory (OOM) for CPUSET
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
	tst_res(TINFO, "OOM on CPUSET...");
	testoom(0, 0, ENOMEM, 1);

	if (is_numa(NULL, NH_MEMS, 2) &&
	    SAFE_CG_HAS(tst_cg, "cpuset.memory_migrate")) {
		/*
		 * Under NUMA system, the migration of cpuset's memory
		 * is in charge of cpuset.memory_migrate, we can write
		 * 1 to cpuset.memory_migrate to enable the migration.
		 */
		SAFE_CG_PRINT(tst_cg, "cpuset.memory_migrate", "1");

		tst_res(TINFO, "OOM on CPUSET with mem migrate:");
		testoom(0, 0, ENOMEM, 1);
	}
}

static void setup(void)
{
	int memnode, ret;

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
	write_cpusets(tst_cg, memnode);
	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.max_runtime = TST_UNLIMITED_RUNTIME,
	.setup = setup,
	.test_all = verify_oom,
	.needs_cgroup_ctrls = (const char *const []){ "cpuset", NULL },
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/vm/overcommit_memory", "1", TST_SR_TBROK},
		{}
	},
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
