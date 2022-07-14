// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Verify that any user can successfully increase the nice value of
 * the process by passing an increment value (< max. applicable limits) to
 * nice() system call.
 */
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>
#include "tst_test.h"

#define	NICEINC	2

static void nice_test(void)
{
	int new_nice;
	int orig_nice;

	orig_nice = SAFE_GETPRIORITY(PRIO_PROCESS, 0);

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

	if (new_nice != (orig_nice + NICEINC)) {
		tst_res(TFAIL, "Process priority %i, expected %i",
		        new_nice, orig_nice + NICEINC);
		return;
	}

	tst_res(TPASS, "nice(%d) passed", NICEINC);

	exit(0);
}

static void verify_nice(void)
{
	if (!SAFE_FORK())
		nice_test();

	tst_reap_children();
}

static struct tst_test test = {
	.forks_child = 1,
	.test_all = verify_nice,
};
