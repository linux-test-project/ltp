// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) Linux Test Project, 2009-2019
 *
 * AUTHORS
 * Paul Larson
 * Matthias Maennich
 *
 * DESCRIPTION
 * Test 1: Suppress handling SIGUSR1 and SIGUSR1, raise them and assert their
 * signal pending.
 * Test 2: Call sigpending(sigset_t*=-1), it should return -1 with errno EFAULT.
 */

#include "config.h"
#include "tst_test.h"
#include "ltp_signal.h"
#include "lapi/syscalls.h"

static void sigpending_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing libc sigpending()");
	break;
	case 1:
		tst_res(TINFO, "Testing __NR_sigpending syscall");
	break;
	case 2:
		tst_res(TINFO, "Testing __NR_rt_sigpending syscall");
	break;
	}
}

static int tested_sigpending(sigset_t *sigset)
{
	switch (tst_variant) {
	case 0:
#ifndef HAVE_SIGPENDING
		tst_brk(TCONF, "libc sigpending() is not implemented");
#else
		return sigpending(sigset);
#endif
	break;
	case 1:
		return tst_syscall(__NR_sigpending, sigset);
	case 2:
		return tst_syscall(__NR_rt_sigpending, sigset, SIGSETSIZE);
	}
	return -1;
}

static int sighandler_counter;
static void sighandler(int signum LTP_ATTRIBUTE_UNUSED)
{
	++sighandler_counter;
}

static void test_sigpending(void)
{
	int SIGMAX = MIN(sizeof(sigset_t) * 8, (size_t)_NSIG);

	int i; /* loop index */

	sighandler_counter = 0;

	/* set up signal mask and handler */
	sigset_t only_SIGUSR, old_mask;
	sighandler_t old_sighandler1, old_sighandler2;
	sigemptyset(&only_SIGUSR);
	sigaddset(&only_SIGUSR, SIGUSR1);
	sigaddset(&only_SIGUSR, SIGUSR2);
	if (sigprocmask(SIG_SETMASK, &only_SIGUSR, &old_mask))
		tst_brk(TBROK, "sigprocmask failed");
	old_sighandler1 = SAFE_SIGNAL(SIGUSR1, sighandler);
	old_sighandler2 = SAFE_SIGNAL(SIGUSR2, sighandler);

	/* Initially no signal should be pending */
	sigset_t pending;
	sigemptyset(&pending);
	TEST(tested_sigpending(&pending));

	for (i = 1; i < SIGMAX; ++i)
		if (sigismember(&pending, i))
			tst_brk(TFAIL,
				"initialization failed: no signal should be pending by now");

	/* raise a signal */
	if (raise(SIGUSR1))
		tst_brk(TBROK, "raising SIGUSR1 failed");
	if (sighandler_counter > 0)
		tst_brk(TFAIL,
			"signal handler is not (yet) supposed to be called");

	/* now we should have exactly one pending signal (SIGUSR1) */
	sigemptyset(&pending);
	TEST(tested_sigpending(&pending));
	for (i = 1; i < SIGMAX; ++i)
		if ((i == SIGUSR1) != sigismember(&pending, i))
			tst_brk(TFAIL, "only SIGUSR1 should be pending by now");

	/* raise another signal */
	if (raise(SIGUSR2))
		tst_brk(TBROK, "raising SIGUSR2 failed");
	if (sighandler_counter > 0)
		tst_brk(TFAIL,
			"signal handler is not (yet) supposed to be called");

	/* now we should have exactly two pending signals (SIGUSR1, SIGUSR2) */
	sigemptyset(&pending);
	TEST(tested_sigpending(&pending));
	for (i = 1; i < SIGMAX; ++i)
		if ((i == SIGUSR1 || i == SIGUSR2) != sigismember(&pending, i))
			tst_brk(TFAIL,
				"only SIGUSR1, SIGUSR2 should be pending by now");

	tst_res(TPASS, "basic sigpending test successful");

	/* reinstate old mask */
	if (sigprocmask(SIG_SETMASK, &old_mask, NULL))
		tst_brk(TBROK, "sigprocmask failed");

	/* at this time the signal handler has been called, once for each signal */
	if (sighandler_counter != 2)
		tst_brk(TFAIL,
			"signal handler has not been called for each signal");

	/* reinstate the original signal handlers */
	SAFE_SIGNAL(SIGUSR1, old_sighandler1);
	SAFE_SIGNAL(SIGUSR2, old_sighandler2);
}

static void test_efault_on_invalid_sigset(void)
{
	/* set sigset to point to an invalid location */
	sigset_t *sigset = tst_get_bad_addr(NULL);

	TEST(tested_sigpending(sigset));

	/* check return code */
	if (TST_RET == -1) {
		if (TST_ERR != EFAULT) {
			tst_res(TFAIL | TTERRNO,
				"syscall failed with wrong errno, expected errno=%d, got %d",
				EFAULT, TST_ERR);
		} else {
			tst_res(TPASS | TTERRNO, "expected failure");
		}
	} else {
		tst_res(TFAIL,
			"syscall failed, expected return value=-1, got %ld",
			TST_RET);
	}
}

static void run(void)
{
	sigpending_info();
	test_sigpending();
	test_efault_on_invalid_sigset();
}

static struct tst_test test = {
	.test_all = run,
	.test_variants = 3,
};
