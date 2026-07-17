// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Copyright (c) 2026 Pavithra <pavrampu@linux.ibm.com>
 */

/*
 * Origin: https://github.com/libhugetlbfs/libhugetlbfs/blob/master/tests/truncate_reserve_wraparound.c
 */

/*\
 * Test that hugepage reservation accounting is correctly maintained when
 * truncating a hugetlbfs file. This verifies that reserved hugepages are
 * properly tracked through the sequence of: mmap (reserves page), touch
 * (instantiates page), truncate (releases page), and attempted access
 * (triggers SIGBUS). A bug in i_size handling could cause the reservation
 * count to become incorrect, leading to resource leaks or allocation failures.
 *
 * Requires root to mount hugetlbfs.
 */

#include <signal.h>
#include <setjmp.h>
#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"

static long hpage_size;
static int fd = -1;

static sigjmp_buf sig_escape;

static void sigbus_handler(int signum LTP_ATTRIBUTE_UNUSED)
{
	siglongjmp(sig_escape, 17);
}

static void run_test(void)
{
	volatile int sigbus_count = 0;
	unsigned long initial_rsvd, after_map_rsvd, after_touch_rsvd;
	unsigned long after_trunc_rsvd, after_unmap_rsvd, after_sigbus_rsvd;
	volatile unsigned int *q;
	void *p;

	initial_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);
	tst_res(TINFO, "Reserve count before map: %lu", initial_rsvd);

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
			fd, 0);
	q = p;

	after_map_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	TST_EXP_EQ_LU(initial_rsvd + 1, after_map_rsvd);
	if (!TST_PASS)
		goto windup;

	*q = 0;
	after_touch_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	TST_EXP_EQ_LU(initial_rsvd, after_touch_rsvd);
	if (!TST_PASS)
		goto windup;

	SAFE_FTRUNCATE(fd, 0);
	after_trunc_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	TST_EXP_EQ_LU(initial_rsvd, after_trunc_rsvd);
	if (!TST_PASS)
		goto windup;

	if (sigsetjmp(sig_escape, 1) == 0)
		*q; /* Fault, triggering a SIGBUS */
	else
		sigbus_count++;

	if (sigbus_count != 1) {
		tst_res(TFAIL, "Didn't SIGBUS after truncate");
		goto windup;
	}

	after_sigbus_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	TST_EXP_EQ_LU(initial_rsvd, after_sigbus_rsvd);
	if (!TST_PASS)
		goto windup;

windup:
	SAFE_MUNMAP(p, hpage_size);
	after_unmap_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	TST_EXP_EQ_LU(initial_rsvd, after_unmap_rsvd);
}

static void setup(void)
{

	hpage_size = tst_get_hugepage_size();
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	struct sigaction sa = {
		.sa_handler = sigbus_handler,
		.sa_flags = 0,
	};

	SAFE_SIGACTION(SIGBUS, &sa, NULL);
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "ebed4bfc8da8"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.hugepages = {1, TST_NEEDS},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
};
