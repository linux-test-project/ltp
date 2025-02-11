// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2003-2022
 */

/*\
 * Verify that root can provide a negative value to nice() system call and hence
 * root can decrease the nice value of the process using nice().
 */

#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include "tst_test.h"

#define MIN_PRIO        -20

static int nice_inc[] = {-1, -12, -50};

static void verify_nice(unsigned int i)
{
	int new_nice;
	int orig_nice;
	int exp_nice;
	int inc = nice_inc[i];
	int delta;

	orig_nice = SAFE_GETPRIORITY(PRIO_PROCESS, 0);

	TEST(nice(inc));

	exp_nice = MAX(MIN_PRIO, (orig_nice + inc));

	if (TST_RET != exp_nice) {
		tst_res(TFAIL | TTERRNO, "nice(%d) returned %li, expected %i",
				inc, TST_RET, exp_nice);
		return;
	}

	if (TST_ERR) {
		tst_res(TFAIL | TTERRNO, "nice(%d) failed", inc);
		return;
	}

	new_nice = SAFE_GETPRIORITY(PRIO_PROCESS, 0);

	if (new_nice != exp_nice) {
		tst_res(TFAIL, "Process priority %i, expected %i",
				new_nice, exp_nice);
		return;
	}

	tst_res(TPASS, "nice(%d) passed", inc);

	delta = orig_nice - exp_nice;
	TEST(nice(delta));
	if (TST_ERR)
		tst_brk(TBROK | TTERRNO, "nice(%d) failed", delta);
}

static struct tst_test test = {
	.needs_root = 1,
	.test = verify_nice,
	.tcnt = ARRAY_SIZE(nice_inc),
};
