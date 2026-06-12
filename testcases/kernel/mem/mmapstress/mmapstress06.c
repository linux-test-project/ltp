// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2003
 * Copyright (c) 2026 Linux Test Project
 */

/*\
 * Test :manpage:`mmap(2)` with swap behavior.
 *
 * Mmap a large anonymous shared region and force it to be swapped out
 * by setting memory limits using Cgroups and dirtying all pages.
 *
 * This test requires root privileges because configuring cgroup memory
 * limits requires administrative access.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "tst_test.h"

static struct tst_cg_group *cg_child;
static int page_size;
static size_t map_size;
static size_t mem_limit;
static pid_t child_pid = -1;

static void run_test(void)
{
	cg_child = tst_cg_group_mk(tst_cg, "child");

	/* Set memory limit to force swapping of the mapping */
	SAFE_CG_PRINTF(cg_child, "memory.max", "%lu", (unsigned long)mem_limit);

	if (TST_CG_VER_IS_V1(cg_child, "memory"))
		SAFE_CG_PRINT(cg_child, "memory.swap.max", "-1");
	else
		SAFE_CG_PRINT(cg_child, "memory.swap.max", "max");

	child_pid = SAFE_FORK();
	if (!child_pid) {
		char *mmapaddr;
		unsigned long cg_swap_before = 0, cg_swap_after = 0;

		/* Move child to the constrained cgroup */
		SAFE_CG_PRINTF(cg_child, "cgroup.procs", "%d", getpid());

		SAFE_CG_SCANF(cg_child, "memory.swap.current", "%lu", &cg_swap_before);

		mmapaddr = SAFE_MMAP(NULL, map_size, PROT_READ | PROT_WRITE,
				     MAP_ANONYMOUS | MAP_SHARED, -1, 0);

		tst_res(TINFO, "Dirtying %zu bytes in child", map_size);

		for (size_t i = 0; i < map_size; i += page_size) {
			mmapaddr[i] = 'a';
			if ((i % (2 * 1024 * 1024)) == 0)
				usleep(1000);
		}

		SAFE_CG_SCANF(cg_child, "memory.swap.current", "%lu", &cg_swap_after);

		if (cg_swap_after > cg_swap_before) {
			tst_res(TPASS, "Cgroup swap usage increased from %lu MB to %lu MB",
				cg_swap_before / TST_MB, cg_swap_after / TST_MB);
		} else {
			tst_res(TFAIL, "Cgroup swap usage did not increase (before: %lu B, after: %lu B)",
				cg_swap_before, cg_swap_after);
		}

		SAFE_MUNMAP(mmapaddr, map_size);
		exit(0);
	}

	SAFE_WAITPID(child_pid, NULL, 0);
	child_pid = -1;

	cg_child = tst_cg_group_rm(cg_child);
}

static void setup(void)
{
	if (!SAFE_CG_HAS(tst_cg, "memory.swap.max"))
		tst_brk(TCONF, "Cgroup swap controller is not enabled/supported");

	page_size = getpagesize();

	mem_limit = 128 * TST_MB;
	map_size = 256 * TST_MB;
}

static void cleanup(void)
{
	if (child_pid > 0) {
		SAFE_KILL(child_pid, SIGKILL);
		SAFE_WAITPID(child_pid, NULL, 0);
	}

	if (cg_child)
		cg_child = tst_cg_group_rm(cg_child);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.forks_child = 1,
	.needs_root = 1,
	.needs_cgroup_ctrls = (const char *const []){ "memory", NULL },
	.min_mem_avail = 256,
	.min_swap_avail = 128,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_SWAP=y",
		"CONFIG_MEMCG=y",
		NULL
	},
};
