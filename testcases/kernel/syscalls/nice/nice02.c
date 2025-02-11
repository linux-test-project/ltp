// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that any user can successfully increase the nice value of
 * the process by passing a higher increment value (> max. applicable limits)
 * to nice() system call.
 */
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>

#include "tst_test.h"

#define	NICEINC 50
#define MAX_PRIO 19
#define DEFAULT_PRIO 0

static void verify_nice(void)
{
	int new_nice;

	TEST(nice(NICEINC));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "nice(%d) returned -1", NICEINC);
		return;
	}

	if (TST_ERR) {
		tst_res(TFAIL | TTERRNO, "nice(%d) failed", NICEINC);
		return;
	}

	new_nice = SAFE_GETPRIORITY(PRIO_PROCESS, 0);

	if (new_nice != MAX_PRIO) {
		tst_res(TFAIL, "Process priority %i, expected %i",
			new_nice, MAX_PRIO);
		return;
	}

	tst_res(TPASS, "nice(%d) passed", NICEINC);

	TEST(nice(DEFAULT_PRIO));
	if (TST_ERR)
		tst_brk(TBROK | TTERRNO, "nice(%d) failed", DEFAULT_PRIO);
}

static struct tst_test test = {
	.test_all = verify_nice,
};
