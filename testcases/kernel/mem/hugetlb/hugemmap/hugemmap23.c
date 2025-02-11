// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation.
 * Copyright (c) Linux Test Project, 2023
 * Author: David Gibson & Adam Litke
 */

/*\
 * This test uses mprotect to change protection of hugepage mapping and
 * perform read/write operation. It checks if the operation results in
 * expected behaviour as per the protection.
 */

#include <setjmp.h>
#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
#define RANDOM_CONSTANT 0x1234ABCD
#define FLAGS_DESC(x) x, #x

static int fd = -1;
static sigjmp_buf sig_escape;
static void *sig_expected = MAP_FAILED;
static long hpage_size;
static void *addr;

static struct tcase {
	char *tname;
	unsigned long len1;
	int prot1;
	char *prot1_str;
	unsigned long len2;
	int prot2;
	char *prot2_str;
} tcases[] = {
	{"R->RW", 1, FLAGS_DESC(PROT_READ), 1, FLAGS_DESC(PROT_READ|PROT_WRITE)},
	{"RW->R", 1, FLAGS_DESC(PROT_READ | PROT_WRITE), 1, FLAGS_DESC(PROT_READ)},
	{"R->RW 1/2", 2, FLAGS_DESC(PROT_READ), 1, FLAGS_DESC(PROT_READ | PROT_WRITE)},
	{"RW->R 1/2", 2, FLAGS_DESC(PROT_READ | PROT_WRITE), 1, FLAGS_DESC(PROT_READ)},
	{"NONE->R", 1, FLAGS_DESC(PROT_NONE), 1, FLAGS_DESC(PROT_READ)},
	{"NONE->RW", 1, FLAGS_DESC(PROT_NONE), 1, FLAGS_DESC(PROT_READ | PROT_WRITE)},
};

static void sig_handler(int signum, siginfo_t *si, void *uc)
{
	(void)uc;

	if (signum == SIGSEGV) {
		tst_res(TINFO, "SIGSEGV at %p (sig_expected=%p)", si->si_addr,
		       sig_expected);
		if (si->si_addr == sig_expected)
			siglongjmp(sig_escape, 1);
		tst_res(TFAIL, "SIGSEGV somewhere unexpected");
	} else {
		tst_res(TFAIL, "Unexpected signal %s", strsignal(signum));
	}
}

static int test_read(void *p)
{
	volatile unsigned long *pl = p;
	unsigned long x;

	if (sigsetjmp(sig_escape, 1)) {
		/* We got a SEGV */
		sig_expected = MAP_FAILED;
		return -1;
	}

	sig_expected = p;
	barrier();
	x = *pl;
	tst_res(TINFO, "Read back %lu", x);
	barrier();
	sig_expected = MAP_FAILED;
	return 0;
}

static int test_write(void *p, unsigned long val)
{
	volatile unsigned long *pl = p;
	unsigned long x;

	if (sigsetjmp(sig_escape, 1)) {
		/* We got a SEGV */
		sig_expected = MAP_FAILED;
		return -1;
	}

	sig_expected = p;
	barrier();
	*pl = val;
	x = *pl;
	barrier();
	sig_expected = MAP_FAILED;

	return (x != val);
}

static int test_prot(void *p, int prot, char *prot_str)
{
	int r, w;

	r = test_read(p);
	tst_res(TINFO, "On Read: %d", r);
	w = test_write(p, RANDOM_CONSTANT);
	tst_res(TINFO, "On Write: %d", w);

	if (prot & PROT_READ) {
		if (r != 0) {
			tst_res(TFAIL, "read failed on mmap(prot %s)", prot_str);
			return -1;
		}
	} else {
		if (r != -1) {
			tst_res(TFAIL, "read succeeded on mmap(prot %s)", prot_str);
			return -1;
		}
	}

	if (prot & PROT_WRITE) {
		switch (w) {
		case -1:
			tst_res(TFAIL, "write failed on mmap(prot %s)", prot_str);
			return -1;
		case 0:
			break;
		case 1:
			tst_res(TFAIL, "write mismatch on mmap(prot %s)", prot_str);
			return -1;
		default:
			tst_res(TWARN, "Bug in test");
			return -1;
		}
	} else {
		switch (w) {
		case -1:
			break;
		case 0:
			tst_res(TFAIL, "write succeeded on mmap(prot %s)", prot_str);
			return -1;
		case 1:
			tst_res(TFAIL, "write mismatch on mmap(prot %s)", prot_str);
			return -1;
		default:
			tst_res(TWARN, "Bug in test");
			break;
		}
	}

	return 0;
}

static void run_test(unsigned int i)
{
	void *p;
	int ret;
	struct tcase *tc = &tcases[i];

	tst_res(TINFO, "Test Name: %s", tc->tname);

	p = SAFE_MMAP(NULL, tc->len1*hpage_size, tc->prot1, MAP_SHARED, fd, 0);

	ret = test_prot(p, tc->prot1, tc->prot1_str);
	if (ret)
		goto cleanup;

	ret = mprotect(p, tc->len2*hpage_size, tc->prot2);
	if (ret != 0) {
		tst_res(TFAIL|TERRNO, "%s: mprotect(prot %s)",
				tc->tname, tc->prot2_str);
		goto cleanup;
	}

	ret = test_prot(p, tc->prot2, tc->prot2_str);
	if (ret)
		goto cleanup;

	if (tc->len2 < tc->len1)
		ret = test_prot(p + tc->len2*hpage_size, tc->prot1, tc->prot1_str);

	tst_res(TPASS, "Successfully tested mprotect %s", tc->tname);

cleanup:
	SAFE_MUNMAP(p, tc->len1*hpage_size);
}

static void setup(void)
{
	struct sigaction sa = {
		.sa_sigaction = sig_handler,
		.sa_flags = SA_SIGINFO,
	};

	hpage_size = tst_get_hugepage_size();
	SAFE_SIGACTION(SIGSEGV, &sa, NULL);

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	addr = SAFE_MMAP(NULL, 2*hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	memset(addr, 0, hpage_size);
	SAFE_MUNMAP(addr, hpage_size);
}

static void cleanup(void)
{
	SAFE_MUNMAP(addr+hpage_size, hpage_size);
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = run_test,
	.hugepages = {2, TST_NEEDS},
};
