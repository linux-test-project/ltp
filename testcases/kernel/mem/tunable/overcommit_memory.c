/*
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
 * When overcommit_memory = 0, allocatable memory can't overextends
 * the amount of free memory. I choose the three cases:
 * a. less than free_total:    free_total / 2, alloc should pass.
 * b. greater than free_total: free_total * 2, alloc should fail.
 * c. equal to sum_total:      sum_tatal,      alloc should fail
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
 * b. greater than commit_left: commit_left * 2, alloc should fail
 * c. overcommit limit:         CommitLimit,     alloc should fail
 * *note: CommitLimit is the current overcommit limit.
 *        Committed_AS is the amount of memory that system has used.
 * it couldn't choose 'equal to commit_left' as a case, because
 * commit_left rely on Committed_AS, but the Committed_AS is not stable.
 *
 * References:
 * - Documentation/sysctl/vm.txt
 * - Documentation/vm/overcommit-accounting
 *
 * ********************************************************************
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * ********************************************************************
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "safe_macros.h"
#include "mem.h"

#define DEFAULT_OVER_RATIO	50L
#define EXPECT_PASS		0
#define EXPECT_FAIL		1

char *TCID = "overcommit_memory";
static long old_overcommit_memory;
static long old_overcommit_ratio;
static long overcommit_ratio;
static long sum_total;
static long free_total;
static long commit_limit;
static long commit_left;
static int R_flag;
static char *R_opt;
option_t options[] = {
	{"R:", &R_flag, &R_opt},
	{NULL, NULL, NULL}
};

static void overcommit_memory_test(void);
static int heavy_malloc(long size);
static void alloc_and_check(long size, int expect_result);
static void usage(void);
static void update_mem(void);

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, options, &usage);

#if __WORDSIZE == 32
	tst_brkm(TCONF, NULL, "test is not designed for 32-bit system.");
#endif

	if (R_flag)
		overcommit_ratio = SAFE_STRTOL(NULL, R_opt, 0, LONG_MAX);
	else
		overcommit_ratio = DEFAULT_OVER_RATIO;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		overcommit_memory_test();
	}

	cleanup();

	tst_exit();
}

void setup(void)
{
	long mem_total, swap_total;
	struct rlimit lim;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	if (access(PATH_SYSVM "overcommit_memory", F_OK) == -1 ||
	    access(PATH_SYSVM "overcommit_ratio", F_OK) == -1)
		tst_brkm(TCONF, NULL, "The system "
			 "can't support to test %s", TCID);

	old_overcommit_memory = get_sys_tune("overcommit_memory");
	old_overcommit_ratio = get_sys_tune("overcommit_ratio");

	mem_total = read_meminfo("MemTotal:");
	tst_resm(TINFO, "MemTotal is %ld kB", mem_total);
	swap_total = read_meminfo("SwapTotal:");
	tst_resm(TINFO, "SwapTotal is %ld kB", swap_total);
	sum_total = mem_total + swap_total;

	commit_limit = read_meminfo("CommitLimit:");
	tst_resm(TINFO, "CommitLimit is %ld kB", commit_limit);

	SAFE_GETRLIMIT(NULL, RLIMIT_AS, &lim);

	if (lim.rlim_cur != RLIM_INFINITY) {
		lim.rlim_cur = RLIM_INFINITY;
		lim.rlim_max = RLIM_INFINITY;

		tst_resm(TINFO, "Increasing RLIM_AS to INFINITY");

		SAFE_SETRLIMIT(NULL, RLIMIT_AS, &lim);
	}

	set_sys_tune("overcommit_ratio", overcommit_ratio, 1);
}

void cleanup(void)
{
	set_sys_tune("overcommit_memory", old_overcommit_memory, 0);
	set_sys_tune("overcommit_ratio", old_overcommit_ratio, 0);
}

static void usage(void)
{
	printf("  -R n    Percentage of overcommitting memory\n");
}

static void overcommit_memory_test(void)
{
	/* start to test overcommit_memory=2 */
	set_sys_tune("overcommit_memory", 2, 1);

	update_mem();
	alloc_and_check(commit_left * 2, EXPECT_FAIL);
	alloc_and_check(commit_limit, EXPECT_FAIL);
	update_mem();
	alloc_and_check(commit_left / 2, EXPECT_PASS);

	/* start to test overcommit_memory=0 */
	set_sys_tune("overcommit_memory", 0, 1);

	update_mem();
	alloc_and_check(free_total / 2, EXPECT_PASS);
	update_mem();
	alloc_and_check(free_total * 2, EXPECT_FAIL);
	alloc_and_check(sum_total, EXPECT_FAIL);

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
		tst_resm(TINFO, "malloc %ld kB successfully", size);
		free(p);
		return 0;
	} else {
		tst_resm(TINFO, "malloc %ld kB failed", size);
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
			tst_resm(TPASS, "alloc passed as expected");
		else
			tst_resm(TFAIL, "alloc failed, expected to pass");
		break;
	case EXPECT_FAIL:
		if (result != 0)
			tst_resm(TPASS, "alloc failed as expected");
		else
			tst_resm(TFAIL, "alloc passed, expected to fail");
		break;
	default:
		tst_brkm(TBROK, cleanup, "Invaild numbler parameter: %d",
			 expect_result);
	}
}

static void update_mem(void)
{
	long mem_free, swap_free;
	long committed;

	mem_free = read_meminfo("MemFree:");
	swap_free = read_meminfo("SwapFree:");
	free_total = mem_free + swap_free;
	commit_limit = read_meminfo("CommitLimit:");

	if (get_sys_tune("overcommit_memory") == 2) {
		committed = read_meminfo("Committed_AS:");
		commit_left = commit_limit - committed;

		if (commit_left < 0) {
			tst_resm(TINFO, "CommitLimit is %ld, Committed_AS"
				 " is %ld", commit_limit, committed);
			tst_brkm(TBROK, cleanup, "Unexpected error: "
				 "CommitLimit < Committed_AS");
		}
	}
}
