// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Copyright (c) 2026 Pavithra <pavrampu@linux.ibm.com>
 */

/*\
 * Test for a bug where truncating hugepage files at offsets >= 4GB on
 * 32-bit systems could incorrectly truncate lower pages due to a
 * misconversion of hugetlb_vmtruncate_list to a prio_tree.
 *
 * This test verifies that:
 * - Pages below 4GB remain accessible after truncation at 4GB
 * - Pages at or above the truncation point correctly trigger SIGBUS
 *
 * WARNING: The offsets and addresses used within are specifically
 * calculated to trigger the bug as it existed. Don't mess with them
 * unless you *really* know what you're doing.
 *
 * Requires root to mount hugetlbfs.
 */

#define _GNU_SOURCE
#define FOURGIG ((off64_t)0x100000000ULL)
#define MNTPOINT "hugetlbfs/"

#include <signal.h>
#include <setjmp.h>
#include "hugetlb.h"

static int page_size;
static long hpage_size;
static int fd = -1;
static long long buggy_offset;
static volatile int test_pass;
static sigjmp_buf sig_escape;

static void sigbus_handler_fail(int signum LTP_ATTRIBUTE_UNUSED,
				siginfo_t *si LTP_ATTRIBUTE_UNUSED,
				void *uc LTP_ATTRIBUTE_UNUSED)
{
	test_pass = -1;
	siglongjmp(sig_escape, 17);
}

static void sigbus_handler_pass(int signum LTP_ATTRIBUTE_UNUSED,
				siginfo_t *si LTP_ATTRIBUTE_UNUSED,
				void *uc LTP_ATTRIBUTE_UNUSED)
{
	test_pass = 1;
	siglongjmp(sig_escape, 17);
}

static void run_test(void)
{
	void *p, *q;
	volatile unsigned int *pi, *qi;

	struct sigaction sa_pass = {
		.sa_sigaction = sigbus_handler_pass,
		.sa_flags = SA_SIGINFO,
	};

	struct sigaction sa_fail = {
		.sa_sigaction = sigbus_handler_fail,
		.sa_flags = SA_SIGINFO,
	};

	test_pass = 0;

	buggy_offset = FOURGIG / (hpage_size / page_size);
	buggy_offset = (long long)PALIGN(buggy_offset, hpage_size);

	/* First get arena of three hpages size, at file offset 4GB */
	q = SAFE_MMAP(NULL, 3*hpage_size, PROT_READ|PROT_WRITE,
		      MAP_PRIVATE, fd, FOURGIG);
	qi = q;
	/* Touch the high page */
	*qi = 0;

	/* This part of the test makes the problem more obvious, but
	 * is not essential.  It can't be done on segmented powerpc, where
	 * segment restrictions prohibit us from performing such a
	 * mapping, so skip it there. Similarly, ia64's address space
	 * restrictions prevent this.
	 */
#if (defined(__powerpc__) && defined(PPC_NO_SEGMENTS)) || \
	!defined(__powerpc__) && !defined(__powerpc64__) && \
	!defined(__ia64__)
	/* Replace middle hpage by tinypage mapping to trigger
	 * nr_ptes BUG
	 */
	p = SAFE_MMAP(q + hpage_size, hpage_size, PROT_READ|PROT_WRITE,
		      MAP_FIXED|MAP_PRIVATE|MAP_ANON, -1, 0);
	pi = p;
	/* Touch one page to allocate its page table */
	*pi = 0;
#endif

	/* Replace top hpage by hpage mapping at confusing file offset */
	p = SAFE_MMAP(q + 2*hpage_size, hpage_size, PROT_READ|PROT_WRITE,
		      MAP_FIXED|MAP_PRIVATE, fd, buggy_offset);
	pi = p;
	/* Touch the low page with something non-zero */
	*pi = 1;

	SAFE_FTRUNCATE(fd, FOURGIG);

	SAFE_SIGACTION(SIGBUS, &sa_fail, NULL);
	if (sigsetjmp(sig_escape, 1) == 0) {
		if (*pi != 1) {
			tst_res(TFAIL, "Data 1 has changed!");
			goto cleanup;
		}
	} else if (test_pass == -1) {
		tst_res(TFAIL, "Unexpected SIGBUS on low-offset page (kernel bug present)");
		goto cleanup;
	}

	SAFE_SIGACTION(SIGBUS, &sa_pass, NULL);
	if (sigsetjmp(sig_escape, 1) == 0) {
		*qi;
		tst_res(TFAIL, "Didn't SIGBUS on truncated page.");
	}

	if (test_pass)
		tst_res(TPASS, "Expected SIGBUS");

cleanup:
	SAFE_MUNMAP(q, 3*hpage_size);
}

static void setup(void)
{
	page_size = getpagesize();
	hpage_size = tst_get_hugepage_size();
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	if (hpage_size > FOURGIG)
		tst_brk(TCONF, "Huge page size is too large!");
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "856fc2950555"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.hugepages = {4, TST_NEEDS},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
};
