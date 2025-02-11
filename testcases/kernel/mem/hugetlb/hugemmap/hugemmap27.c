// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2013 LG Electronics.
 * Author: Joonsoo Kim
 */

/*\
 * Test to preserve a reserved page against no-reserved mapping. If all
 * hugepages are reserved, access to no-reserved shared mapping cause a
 * process die, instead of stealing a hugepage which is reserved for other
 * process.
 */

#include <setjmp.h>
#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
static long hpage_size;
static int fd1 = -1, fd2 = -1;
static sigjmp_buf sig_escape;
static void *sig_expected = MAP_FAILED;

static void sig_handler(int signum, siginfo_t *si, void *uc)
{
	(void)uc;

	if (signum == SIGBUS) {
		if (si->si_addr == sig_expected)
			siglongjmp(sig_escape, 1);
		tst_res(TFAIL, "SIGBUS somewhere unexpected: %p (expected: %p)",
				si->si_addr, sig_expected);
	} else {
		tst_res(TFAIL, "Unexpected signal %s", strsignal(signum));
	}
}

static int test_write(void *p)
{
	volatile char *pl = p;

	if (sigsetjmp(sig_escape, 1)) {
		/* We got a SIGBUS */
		return 1;
	}

	sig_expected = p;
	barrier();
	*pl = 's';
	return 0;
}

static void run_test(void)
{
	int nr_hugepages;
	int surp_hugepages;
	int ret;
	char *p, *q;
	struct sigaction sa = {
		.sa_sigaction = sig_handler,
		.sa_flags = SA_SIGINFO,
	};

	nr_hugepages = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);

	SAFE_SIGACTION(SIGBUS, &sa, NULL);

	p = SAFE_MMAP(NULL, hpage_size * nr_hugepages,
		PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);

	tst_res(TINFO, "Reserve all hugepages %d", nr_hugepages);

	q = SAFE_MMAP(NULL, hpage_size,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fd2, 0);

	tst_res(TINFO, "Write to %p to steal reserved page", q);

	surp_hugepages = SAFE_READ_MEMINFO(MEMINFO_HPAGE_SURP);
	ret = test_write(q);
	if (ret == 1) {
		tst_res(TPASS, "Successful with SIGSEGV received");
		goto cleanup;
	}

	/* Provisioning succeeded because of overcommit */
	if (SAFE_READ_MEMINFO(MEMINFO_HPAGE_SURP) ==
			surp_hugepages + 1) {
		tst_res(TPASS, "Successful because of surplus pages");
		goto cleanup;
	}

	tst_res(TFAIL, "Steal reserved page");
cleanup:
	SAFE_MUNMAP(p, hpage_size * nr_hugepages);
	SAFE_MUNMAP(q, hpage_size);
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();
	fd1 = tst_creat_unlinked(MNTPOINT, 0, 0600);
	fd2 = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	if (fd1 >= 0)
		SAFE_CLOSE(fd1);
	if (fd2 >= 0)
		SAFE_CLOSE(fd2);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {2, TST_NEEDS},
};
