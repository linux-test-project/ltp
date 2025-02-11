// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Allocate anonymous memory pages inside child and reclaim it with
 * MADV_PAGEOUT. Then check if memory pages have been swapped out by looking
 * at smaps information.
 *
 * The advice might be ignored for some pages in the range when it is
 * not applicable, so test passes if swap memory increases after
 * reclaiming memory with MADV_PAGEOUT.
 */

#define _GNU_SOURCE

#include <sys/mman.h>
#include "tst_test.h"
#include "lapi/mmap.h"
#include "lapi/syscalls.h"
#include "process_madvise.h"

#define MEM_LIMIT   (100 * TST_MB)
#define MEMSW_LIMIT (200 * TST_MB)
#define MEM_CHILD   (1   * TST_MB)

static void **data_ptr;

static void child_alloc(void)
{
	char data[MEM_CHILD];
	struct addr_mapping map_before;
	struct addr_mapping map_after;

	memset(data, 'a', MEM_CHILD);

	tst_res(TINFO, "Allocate memory: %d bytes", MEM_CHILD);

	*data_ptr = SAFE_MMAP(NULL, MEM_CHILD,
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	memset(*data_ptr, 'a', MEM_CHILD);

	memset(&map_before, 0, sizeof(struct addr_mapping));
	read_address_mapping((unsigned long)*data_ptr, &map_before);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	memset(&map_after, 0, sizeof(struct addr_mapping));
	read_address_mapping((unsigned long)*data_ptr, &map_after);

	if (memcmp(*data_ptr, data, MEM_CHILD) != 0) {
		tst_res(TFAIL, "Dirty memory after reclaiming it");
		return;
	}

	SAFE_MUNMAP(*data_ptr, MEM_CHILD);
	*data_ptr = NULL;

	TST_EXP_EXPR(map_before.swap < map_after.swap,
		"Most of the memory has been swapped out: %dkB out of %dkB",
		map_after.swap - map_before.swap,
		MEM_CHILD / TST_KB);
}

static void setup(void)
{
	SAFE_CG_PRINTF(tst_cg, "memory.max", "%d", MEM_LIMIT);
	if (SAFE_CG_HAS(tst_cg, "memory.swap.max"))
		SAFE_CG_PRINTF(tst_cg, "memory.swap.max", "%d", MEMSW_LIMIT);

	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());

	data_ptr = SAFE_MMAP(NULL, sizeof(void *),
			PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (*data_ptr)
		SAFE_MUNMAP(*data_ptr, MEM_CHILD);

	if (data_ptr)
		SAFE_MUNMAP(data_ptr, sizeof(void *));
}

static void run(void)
{
	int ret;
	int pidfd;
	pid_t pid_alloc;
	struct iovec vec;

	pid_alloc = SAFE_FORK();
	if (!pid_alloc) {
		child_alloc();
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "Reclaim memory using MADV_PAGEOUT");

	pidfd = SAFE_PIDFD_OPEN(pid_alloc, 0);

	vec.iov_base = *data_ptr;
	vec.iov_len = MEM_CHILD;

	ret = tst_syscall(__NR_process_madvise, pidfd, &vec, 1UL,
			MADV_PAGEOUT, 0UL);

	if (ret == -1)
		tst_brk(TBROK | TERRNO, "process_madvise failed");

	if (ret != MEM_CHILD)
		tst_brk(TBROK, "process_madvise reclaimed only %d bytes", ret);

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.forks_child = 1,
	.min_kver = "5.10",
	.needs_checkpoints = 1,
	.needs_root = 1,
	.min_mem_avail = 2 * MEM_LIMIT / TST_MB,
	.min_swap_avail = 2 * MEM_CHILD / TST_MB,
	.needs_cgroup_ctrls = (const char *const []){ "memory", NULL },
	.needs_kconfigs = (const char *[]) {
		"CONFIG_SWAP=y",
		NULL
	},
};
