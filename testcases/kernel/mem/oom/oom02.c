// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Out Of Memory (OOM) for mempolicy - need NUMA system support
 *
 * The program is designed to cope with unpredictable like amount and
 * system physical memory, swap size and other VMM technology like KSM,
 * memcg, memory hotplug and so on which may affect the OOM
 * behaviours. It simply increase the memory consumption 3G each time
 * until all the available memory is consumed and OOM is triggered.
 *
 * Copyright (C) 2010-2017  Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2011-2023
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
	tst_res(TINFO, "OOM on MPOL_BIND mempolicy...");
	testoom(MPOL_BIND, 0, ENOMEM, 1);

	tst_res(TINFO, "OOM on MPOL_INTERLEAVE mempolicy...");
	testoom(MPOL_INTERLEAVE, 0, ENOMEM, 1);

	tst_res(TINFO, "OOM on MPOL_PREFERRED mempolicy...");
	testoom(MPOL_PREFERRED, 0, ENOMEM, 1);
}

static void setup(void)
{
	if (!is_numa(NULL, NH_MEMS, 2))
		tst_brk(TCONF, "The case need a NUMA system.");
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.max_runtime = TST_UNLIMITED_RUNTIME,
	.setup = setup,
	.test_all = verify_oom,
	.skip_in_compat = 1,
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/vm/overcommit_memory", "1", TST_SR_TBROK},
		{}
	},
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
