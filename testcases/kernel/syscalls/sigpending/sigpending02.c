// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (c) International Business Machines  Corp., 2002
 *
 * AUTHORS
 *	Paul Larson
 *
 * DESCRIPTION
 *	Test to see that the proper errors are returned by sigpending. All the
 *	tests can also be compiled to use the rt_sigpending syscall instead. To
 *	simplify the documentation, only sigpending() is usually mentioned
 *	below.
 *
 *	Test 1:
 *		Call sigpending(sigset_t*=-1), it should return -1 with errno EFAULT
 */

#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "tst_test.h"
#include "ltp_signal.h"
#include "lapi/syscalls.h"

static void run(void)
{
	/* set sigset to point to an invalid location */
	sigset_t *sigset = (sigset_t *) - 1;

#if defined(TEST_SIGPENDING)
	TEST(tst_syscall(__NR_sigpending, sigset));
#elif defined(TEST_RT_SIGPENDING)
	TEST(tst_syscall(__NR_rt_sigpending, sigset, SIGSETSIZE));
#else
#error Neither TEST_SIGPENDING nor TEST_RT_SIGPENDING is defined!
#endif

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

static struct tst_test test = {
	.test_all = run
};
