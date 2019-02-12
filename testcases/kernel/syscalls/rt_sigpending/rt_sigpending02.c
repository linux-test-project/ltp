// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (c) International Business Machines  Corp., 2002
 *
 * AUTHORS
 *	Paul Larson
 *
 * DESCRIPTION
 *	Test to see that the proper errors are returned by rt_sigpending
 *
 *	Test 1:
 *		Call rt_sigpending(sigset_t*=-1, SIGSETSIZE),
 *		it should return -1 with errno EFAULT
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

	TEST(tst_syscall(__NR_rt_sigpending, sigset, SIGSETSIZE));

	/* check return code */
	if (TST_RET == -1) {
		if (TST_ERR != EFAULT) {
			tst_res(TFAIL | TTERRNO,
				"rt_sigpending() Failed with wrong errno, "
				"expected errno=%d, got ", EFAULT);
		} else {
			tst_res(TPASS | TTERRNO, "expected failure");
		}
	} else {
		tst_res(TFAIL,
			"rt_sigpending() Failed, expected return value=-1, got %ld", TST_RET);
	}
}

static struct tst_test test = {
	.test_all = run
};
