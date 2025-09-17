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
 *
 * Ensure that memory allocated with MAP_DROPPABLE can be reclaimed
 * under memory pressure within a cgroup.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/mmap.h"
#include "tst_safe_macros.h"

#define MEM_LIMIT (256 * TST_MB)
#define ALLOC_SIZE (128 * TST_MB)

static struct tst_cg_group *cg_child;
static pid_t child;
static int page_size;

static void stress_child(void)
{
	for (;;) {
		char *buf = malloc(page_size);

		if (!buf)
			exit(1);
		memset(buf, 'B', page_size);
	}
}

static void test_mmap(void)
{
	char *alloc;
	unsigned char *vec;
	size_t npages = ALLOC_SIZE / page_size;

	cg_child = tst_cg_group_mk(tst_cg, "child");
	SAFE_CG_PRINTF(tst_cg, "memory.max", "%d", MEM_LIMIT);
	if (!TST_CG_VER_IS_V1(tst_cg, "memory"))
		SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%d", 0);
	else
		SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%d", MEM_LIMIT);
	SAFE_CG_PRINTF(cg_child, "cgroup.procs", "%d", getpid());

	alloc = SAFE_MMAP(0, ALLOC_SIZE, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_DROPPABLE, -1, 0);

	memset(alloc, 'A', ALLOC_SIZE);

	vec = SAFE_MALLOC(npages);

	child = SAFE_FORK();
	if (!child)
		stress_child();

	for (;;) {
		if (!tst_remaining_runtime()) {
			tst_res(TFAIL, "MAP_DROPPABLE did not drop pages within timeout");
			goto cleanup;
		}

		SAFE_MINCORE(alloc, ALLOC_SIZE, vec);

		for (size_t i = 0; i < npages; i++) {
			if (!(vec[i] & 1)) {
				tst_res(TPASS, "MAP_DROPPABLE page reclaimed by kernel");
				goto cleanup;
			}
		}

		usleep(100000);
	}

cleanup:
	if (child > 0) {
		SAFE_KILL(child, SIGKILL);
		SAFE_WAITPID(child, NULL, 0);
	}
	SAFE_MUNMAP(alloc, ALLOC_SIZE);
	free(vec);
	SAFE_CG_PRINTF(tst_cg_drain, "cgroup.procs", "%d", getpid());
	cg_child = tst_cg_group_rm(cg_child);
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
	page_size = getpagesize();
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
