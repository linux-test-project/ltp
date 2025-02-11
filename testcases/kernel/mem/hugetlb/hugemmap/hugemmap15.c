// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Copyright (c) Linux Test Project, 2022-2023
 * Author: David Gibson & Adam Litke
 */

/*\
 * Older ppc64 kernels don't properly flush dcache to icache before
 * giving a cleared page to userspace.  With some exceedingly
 * hairy code, this attempts to test for this bug.
 *
 * This test will never trigger (obviously) on machines with coherent
 * icache and dcache (including x86 and POWER5).  On any given run,
 * even on a buggy kernel there's a chance the bug won't trigger -
 * either because we don't get the same physical page back when we
 * remap, or because the icache happens to get flushed in the interim.
 */

#if defined(__clang__)
	#pragma clang optimize off
#endif

#define _GNU_SOURCE
#include "hugetlb.h"

#if defined(__powerpc__) || defined(__powerpc64__) || defined(__ia64__) || \
	defined(__s390__) || defined(__s390x__) || defined(__sparc__) || \
	defined(__aarch64__) || (defined(__riscv) && __riscv_xlen == 64) || \
	defined(__i386__) || defined(__x86_64__) || defined(__arm__)

#include <setjmp.h>

#define SUCC_JMP 1
#define FAIL_JMP 2
#define COPY_SIZE	128

/* Seems to be enough to trigger reliably */
#define NUM_REPETITIONS	64
#define MNTPOINT "hugetlbfs/"
static long hpage_size;
static int  fd = -1;

static void cacheflush(void *p)
{
#if defined(__powerpc__)
	asm volatile("dcbst 0,%0; sync; icbi 0,%0; isync" : : "r"(p));
#elif defined(__arm__) || defined(__aarch64__) || defined(__riscv)
	__clear_cache(p, p + COPY_SIZE);
#else
	(void)p;
#endif
}

static void jumpfunc(int copy, void *p)
{
	/*
	 * gcc bug workaround: if there is exactly one &&label
	 * construct in the function, gcc assumes the computed goto
	 * goes there, leading to the complete elision of the goto in
	 * this case
	 */
	void *l = &&dummy;

	l = &&jumplabel;

	if (copy) {
		memcpy(p, l, COPY_SIZE);
		cacheflush(p);
	}

	goto *p;
 dummy:
	tst_res(TWARN, "unreachable?");

 jumplabel:
	return;
}

static sigjmp_buf sig_escape;
static void *sig_expected;

static void sig_handler(int signum, siginfo_t *si, void *uc)
{
#if defined(__powerpc__) || defined(__powerpc64__) || defined(__ia64__) || \
	defined(__s390__) || defined(__s390x__) || defined(__sparc__) || \
	defined(__aarch64__) || (defined(__riscv) && __riscv_xlen == 64)
	/* On powerpc, ia64, s390 and Aarch64, 0 bytes are an illegal
	 * instruction, so, if the icache is cleared properly, we SIGILL
	 * as soon as we jump into the cleared page
	 */
	if (signum == SIGILL) {
		tst_res(TINFO, "SIGILL at %p (sig_expected=%p)", si->si_addr,
				sig_expected);
		if (si->si_addr == sig_expected)
			siglongjmp(sig_escape, SUCC_JMP);
		siglongjmp(sig_escape, FAIL_JMP + SIGILL);
	}
#elif defined(__i386__) || defined(__x86_64__) || defined(__arm__)
	/* On x86, zero bytes form a valid instruction:
	 *	add %al,(%eax)		(i386)
	 * or	add %al,(%rax)		(x86_64)
	 *
	 * So, behaviour depends on the contents of [ER]AX, which in
	 * turn depends on the details of code generation.  If [ER]AX
	 * contains a valid pointer, we will execute the instruction
	 * repeatedly until we run off that hugepage and get a SIGBUS
	 * on the second, truncated page.  If [ER]AX does not contain
	 * a valid pointer, we will SEGV on the first instruction in
	 * the cleared page.  We check for both possibilities
	 * below.
	 *
	 * On 32 bit ARM, zero bytes are interpreted as follows:
	 *  andeq	r0, r0, r0	(ARM state, 4 bytes)
	 *  movs	r0, r0		(Thumb state, 2 bytes)
	 *
	 * So, we only expect to run off the end of the huge page and
	 * generate a SIGBUS.
	 */
	if (signum == SIGBUS) {
		tst_res(TINFO, "SIGBUS at %p (sig_expected=%p)", si->si_addr,
				sig_expected);
		if (sig_expected
		    && (PALIGN(sig_expected, hpage_size)
			== si->si_addr)) {
			siglongjmp(sig_escape, SUCC_JMP);
		}
		siglongjmp(sig_escape, FAIL_JMP + SIGBUS);
	}
#if defined(__x86_64__) || defined(__i386__)
	if (signum == SIGSEGV) {
#ifdef __x86_64__
		void *pc = (void *)((ucontext_t *)uc)->uc_mcontext.gregs[REG_RIP];
#else
		void *pc = (void *)((ucontext_t *)uc)->uc_mcontext.gregs[REG_EIP];
#endif
		tst_res(TINFO, "SIGSEGV at %p, PC=%p (sig_expected=%p)",
				si->si_addr, pc, sig_expected);
		if (sig_expected == pc)
			siglongjmp(sig_escape, SUCC_JMP);
		siglongjmp(sig_escape, FAIL_JMP + SIGSEGV);
	}
#endif
#endif
}

static int test_once(int fd)
{
	void *p, *q;

	SAFE_FTRUNCATE(fd, 0);

	switch (sigsetjmp(sig_escape, 1)) {
	case SUCC_JMP:
		sig_expected = NULL;
		SAFE_FTRUNCATE(fd, 0);
		return 0;
	case FAIL_JMP + SIGILL:
		tst_res(TFAIL, "SIGILL somewhere unexpected");
		return -1;
	case FAIL_JMP + SIGBUS:
		tst_res(TFAIL, "SIGBUS somewhere unexpected");
		return -1;
	case FAIL_JMP + SIGSEGV:
		tst_res(TFAIL, "SIGSEGV somewhere unexpected");
		return -1;
	default:
		break;
	}
	p = SAFE_MMAP(NULL, 2*hpage_size, PROT_READ|PROT_WRITE|PROT_EXEC,
		 MAP_SHARED, fd, 0);

	SAFE_FTRUNCATE(fd, hpage_size);

	q = p + hpage_size - COPY_SIZE;

	jumpfunc(1, q);

	SAFE_FTRUNCATE(fd, 0);
	p = SAFE_MMAP(p, hpage_size, PROT_READ|PROT_WRITE|PROT_EXEC,
		 MAP_SHARED|MAP_FIXED, fd, 0);

	q = p + hpage_size - COPY_SIZE;
	sig_expected = q;

	jumpfunc(0, q); /* This should blow up */

	tst_res(TFAIL, "icache unclean");
	return -1;
}

static void run_test(void)
{
	int i;

	struct sigaction sa = {
		.sa_sigaction = sig_handler,
		.sa_flags = SA_SIGINFO,
	};

	SAFE_SIGACTION(SIGILL, &sa, NULL);
	SAFE_SIGACTION(SIGBUS, &sa, NULL);
	SAFE_SIGACTION(SIGSEGV, &sa, NULL);

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	for (i = 0; i < NUM_REPETITIONS; i++)
		if (test_once(fd))
			goto cleanup;

	tst_res(TPASS, "Successfully tested dcache to icache flush");
cleanup:
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:")*1024;
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "cbf52afdc0eb"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {3, TST_NEEDS},
};
#else
	TST_TEST_TCONF("Signal handler for this architecture hasn't been written");
#endif
