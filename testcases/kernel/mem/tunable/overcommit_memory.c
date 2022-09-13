// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2012-2020 Linux Test Project
 * Copyright (c) 2012-2017 Red Hat, Inc.
 *
 * There are two tunables overcommit_memory and overcommit_ratio under
 * /proc/sys/vm/, which can control memory overcommitment.
 *
 * The overcommit_memory contains a flag that enables memory
 * overcommitment, it has three values:
 * - When this flag is 0, the kernel attempts to estimate the amount
 *   of free memory left when userspace requests more memory.
 * - When this flag is 1, the kernel pretends there is always enough
 *   memory until it actually runs out.
 * - When this flag is 2, the kernel uses a "never overcommit" policy
 *   that attempts to prevent any overcommit of memory.
 *
 * The overcommit_ratio tunable defines the amount by which the kernel
 * overextends its memory resources in the event that overcommit_memory
 * is set to the value of 2. The value in this file represents a
 * percentage added to the amount of actual RAM in a system when
 * considering whether to grant a particular memory request.
 * The general formula for this tunable is:
 * CommitLimit = SwapTotal + MemTotal * overcommit_ratio
 * CommitLimit, SwapTotal and MemTotal can read from /proc/meminfo.
 *
 * The program is designed to test the two tunables:
 *
 * When overcommit_memory = 0, allocatable memory can't overextend
 * the amount of total memory:
 * a. less than free_total:    free_total / 2, alloc should pass.
 * b. greater than sum_total:   sum_total * 2, alloc should fail.
 *
 * When overcommit_memory = 1, it can alloc enough much memory, I
 * choose the three cases:
 * a. less than sum_total:    sum_total / 2, alloc should pass
 * b. equal to sum_total:     sum_total,     alloc should pass
 * c. greater than sum_total: sum_total * 2, alloc should pass
 * *note: sum_total = SwapTotal + MemTotal
 *
 * When overcommit_memory = 2, the total virtual address space on
 * the system is limited to CommitLimit(Swap+RAM*overcommit_ratio)
 * commit_left(allocatable memory) = CommitLimit - Committed_AS
 * a. less than commit_left:    commit_left / 2, alloc should pass
 * b. overcommit limit:         CommitLimit + TotalBatchSize, should fail
 * c. greater than commit_left: commit_left * 2, alloc should fail
 * *note: CommitLimit is the current overcommit limit.
 *        Committed_AS is the amount of memory that system has used.
 * it couldn't choose 'equal to commit_left' as a case, because
 * commit_left rely on Committed_AS, but the Committed_AS is not stable.
 * *note2: TotalBatchSize is the total number of bytes, that can be
 *         accounted for in the per cpu counters for the vm_committed_as
 *         counter. Since the check used by malloc only looks at the
 *         global counter of vm_committed_as, it can overallocate a bit.
 *
 * References:
 * - Documentation/sysctl/vm.txt
 * - Documentation/vm/overcommit-accounting
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "lapi/abisize.h"
#include "mem.h"

#define DEFAULT_OVER_RATIO	50L
#define EXPECT_PASS		0
#define EXPECT_FAIL		1

static char *R_opt;
static long old_overcommit_memory = -1;
static long old_overcommit_ratio = -1;
static long overcommit_ratio;
static long sum_total;
static long free_total;
static long commit_limit;
static long commit_left;
static long total_batch_size;

static int heavy_malloc(long size);
static void alloc_and_check(long size, int expect_result);
static void update_mem(void);
static void update_mem_commit(void);
static void calculate_total_batch_size(void);

static void setup(void)
{
	long mem_total, swap_total;
	struct rlimit lim;

	if (access(PATH_SYSVM "overcommit_memory", F_OK) == -1 ||
	    access(PATH_SYSVM "overcommit_ratio", F_OK) == -1)
		tst_brk(TCONF, "system doesn't support overcommit_memory");

	if (R_opt)
		overcommit_ratio = SAFE_STRTOL(R_opt, 0, LONG_MAX);
	else
		overcommit_ratio = DEFAULT_OVER_RATIO;

	old_overcommit_memory = get_sys_tune("overcommit_memory");
	old_overcommit_ratio = get_sys_tune("overcommit_ratio");

	mem_total = SAFE_READ_MEMINFO("MemTotal:");
	tst_res(TINFO, "MemTotal is %ld kB", mem_total);
	swap_total = SAFE_READ_MEMINFO("SwapTotal:");
	tst_res(TINFO, "SwapTotal is %ld kB", swap_total);
	sum_total = mem_total + swap_total;

	commit_limit = SAFE_READ_MEMINFO("CommitLimit:");
	tst_res(TINFO, "CommitLimit is %ld kB", commit_limit);

	SAFE_GETRLIMIT(RLIMIT_AS, &lim);

	if (lim.rlim_cur != RLIM_INFINITY) {
		lim.rlim_cur = RLIM_INFINITY;
		lim.rlim_max = RLIM_INFINITY;

		tst_res(TINFO, "Increasing RLIM_AS to INFINITY");

		SAFE_SETRLIMIT(RLIMIT_AS, &lim);
	}

	set_sys_tune("overcommit_ratio", overcommit_ratio, 1);

	calculate_total_batch_size();
	tst_res(TINFO, "TotalBatchSize is %ld kB", total_batch_size);
}

static void cleanup(void)
{
	if (old_overcommit_memory != -1)
		set_sys_tune("overcommit_memory", old_overcommit_memory, 0);
	if (old_overcommit_ratio != -1)
		set_sys_tune("overcommit_ratio", old_overcommit_ratio, 0);
}

static void overcommit_memory_test(void)
{

#ifdef TST_ABI32
	tst_brk(TCONF, "test is not designed for 32-bit system.");
#endif
	/* start to test overcommit_memory=2 */
	set_sys_tune("overcommit_memory", 2, 1);

	update_mem_commit();
	alloc_and_check(commit_left * 2, EXPECT_FAIL);
	alloc_and_check(commit_limit + total_batch_size, EXPECT_FAIL);
	update_mem_commit();
	alloc_and_check(commit_left / 2, EXPECT_PASS);

	/* start to test overcommit_memory=0 */
	set_sys_tune("overcommit_memory", 0, 1);

	update_mem();
	alloc_and_check(free_total / 2, EXPECT_PASS);
	alloc_and_check(sum_total * 2, EXPECT_FAIL);

	/* start to test overcommit_memory=1 */
	set_sys_tune("overcommit_memory", 1, 1);

	alloc_and_check(sum_total / 2, EXPECT_PASS);
	alloc_and_check(sum_total, EXPECT_PASS);
	alloc_and_check(sum_total * 2, EXPECT_PASS);

}

static int heavy_malloc(long size)
{
	char *p;

	p = malloc(size * KB);
	if (p != NULL) {
		tst_res(TINFO, "malloc %ld kB successfully", size);
		free(p);
		return 0;
	} else {
		tst_res(TINFO, "malloc %ld kB failed", size);
		return 1;
	}
}

static void alloc_and_check(long size, int expect_result)
{
	int result;

	/* try to alloc size kB memory */
	result = heavy_malloc(size);

	switch (expect_result) {
	case EXPECT_PASS:
		if (result == 0)
			tst_res(TPASS, "alloc passed as expected");
		else
			tst_res(TFAIL, "alloc failed, expected to pass");
		break;
	case EXPECT_FAIL:
		if (result != 0)
			tst_res(TPASS, "alloc failed as expected");
		else
			tst_res(TFAIL, "alloc passed, expected to fail");
		break;
	default:
		tst_brk(TBROK, "Invalid number parameter: %d",
			 expect_result);
	}
}

static void update_mem(void)
{
	long mem_free, swap_free;

	mem_free = SAFE_READ_MEMINFO("MemFree:");
	swap_free = SAFE_READ_MEMINFO("SwapFree:");
	free_total = mem_free + swap_free;
}

static void update_mem_commit(void)
{
	long committed;

	commit_limit = SAFE_READ_MEMINFO("CommitLimit:");
	committed = SAFE_READ_MEMINFO("Committed_AS:");
	commit_left = commit_limit - committed;

	if (commit_left < 0) {
		tst_res(TINFO, "CommitLimit is %ld, Committed_AS is %ld",
			commit_limit, committed);

		if (overcommit_ratio > old_overcommit_ratio) {
			tst_brk(TBROK, "Unexpected error: "
				"CommitLimit < Committed_AS");
		}

		tst_brk(TCONF, "Specified overcommit_ratio %ld <= default %ld, "
			"so it's possible for CommitLimit < Committed_AS and skip test",
			overcommit_ratio, old_overcommit_ratio);
	}
}

static void calculate_total_batch_size(void)
{
	struct sysinfo info;
	long ncpus = tst_ncpus_conf();
	long pagesize = getpagesize();
	SAFE_SYSINFO(&info);

	/* see linux source mm/mm_init.c mm_compute_batch() (This is in pages) */
	long batch_size = MAX(ncpus * 2L,
	                      MAX(32L,
	                          MIN((long)INT32_MAX,
	                              (long)(info.totalram / pagesize) / ncpus / 256
	                          )
	                      )
	                  );

	/* there are ncpu separate counters, that can all grow up to
	 * batch_size. So the maximum error for __vm_enough_memory is
	 * batch_size * ncpus. */
	total_batch_size = (batch_size * ncpus * pagesize) / KB;
}

static struct tst_test test = {
	.needs_root = 1,
	.options = (struct tst_option[]) {
		{"R:", &R_opt, "Percentage of overcommitting memory"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = overcommit_memory_test,
};
