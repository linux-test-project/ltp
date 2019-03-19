// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (c) International Business Machines  Corp., 2002
 *
 * AUTHORS
 *	Paul Larson
 *	Matthias Maennich
 *
 * DESCRIPTION
 *	Test to assert basic functionality of sigpending. All the tests can also be
 *	compiled to use the rt_sigpending syscall instead. To simplify the
 *	documentation, only sigpending() is usually mentioned below.
 *
 *	Test 1:
 *		Suppress handling SIGUSR1 and SIGUSR1, raise them and assert their
 *		signal pending.
 *
 *	Test 2:
 *		Call sigpending(sigset_t*=-1), it should return -1 with errno EFAULT
 */

#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "tst_test.h"
#include "ltp_signal.h"
#include "lapi/syscalls.h"

#if defined(TEST_SIGPENDING)
#define tested_sigpending(sigset) TEST(tst_syscall(__NR_sigpending, sigset))
#elif defined(TEST_RT_SIGPENDING)
#define tested_sigpending(sigset)                                              \
	TEST(tst_syscall(__NR_rt_sigpending, sigset, SIGSETSIZE))
#else
#error Neither TEST_SIGPENDING nor TEST_RT_SIGPENDING is defined!
#endif

static int sighandler_counter;
static void sighandler(int signum LTP_ATTRIBUTE_UNUSED)
{
	++sighandler_counter;
}

static void test_sigpending(void)
{
	int SIGMAX = MIN(sizeof(sigset_t) * 8, (size_t)_NSIG);

	int i; /* loop index */

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
	tested_sigpending(&pending);

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
	tested_sigpending(&pending);
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
	tested_sigpending(&pending);
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

	tested_sigpending(sigset);

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
	test_sigpending();
	test_efault_on_invalid_sigset();
}

static struct tst_test test = {
	.test_all = run
};
