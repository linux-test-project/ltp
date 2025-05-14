// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Wei Gao <wegao@suse.com>
 */

/*\
 * [Description]
 *
 * Test mmap(2) with MAP_DROPPABLE flag.
 *
 * Test base on kernel selftests/mm/droppable.c
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include "tst_test.h"
#include "lapi/mmap.h"

#define MEM_LIMIT (256 * TST_MB)
#define ALLOC_SIZE (128 * TST_MB)

static struct tst_cg_group *cg_child;

static void test_mmap(void)
{
	size_t alloc_size = ALLOC_SIZE;
	size_t page_size = getpagesize();
	char *alloc;
	pid_t child;

	cg_child = tst_cg_group_mk(tst_cg, "child");
	SAFE_CG_PRINTF(tst_cg, "memory.max", "%d", MEM_LIMIT);
	if (!TST_CG_VER_IS_V1(tst_cg, "memory"))
		SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%d", 0);
	else
		SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%d", MEM_LIMIT);
	SAFE_CG_PRINTF(cg_child, "cgroup.procs", "%d", getpid());

	alloc = SAFE_MMAP(0, alloc_size, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_DROPPABLE, -1, 0);

	memset(alloc, 'A', alloc_size);
	for (size_t i = 0; i < alloc_size; i += page_size) {
		if (alloc[i]  != 'A')
			tst_res(TFAIL, "memset failed");
	}

	child = SAFE_FORK();
	if (!child) {
		for (;;)
			*(char *)malloc(page_size) = 'B';
	}

	while (1) {
		for (size_t i = 0; i < alloc_size; i += page_size) {
			if (!tst_remaining_runtime()) {
				tst_res(TFAIL, "MAP_DROPPABLE did not drop memory within the timeout period.");
				goto kill_child;
			}
			if (!alloc[i]) {
				tst_res(TPASS, "MAP_DROPPABLE test pass.");
				goto kill_child;
			}
		}
	}

kill_child:
	SAFE_KILL(child, SIGKILL);
	SAFE_WAITPID(child, NULL, 0);
	SAFE_MUNMAP(alloc, alloc_size);
}

static void setup(void)
{
	void *addr = mmap(0, 1, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_DROPPABLE, -1, 0);

	if (addr == MAP_FAILED && errno == EINVAL)
		tst_brk(TCONF, "MAP_DROPPABLE not supported");

	if (addr == MAP_FAILED)
		tst_brk(TBROK | TERRNO, "mmap() MAP_DROPPABLE failed");

	SAFE_MUNMAP(addr, 1);
}

static void cleanup(void)
{
	if (cg_child) {
		SAFE_CG_PRINTF(tst_cg_drain, "cgroup.procs", "%d", getpid());
		cg_child = tst_cg_group_rm(cg_child);
	}
}

static struct tst_test test = {
	.test_all = test_mmap,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.needs_cgroup_ctrls = (const char *const []){ "memory", NULL },
	.needs_root = 1,
	.cleanup = cleanup,
	.setup = setup,
	.runtime = 30,
	.min_mem_avail = 300,
};
