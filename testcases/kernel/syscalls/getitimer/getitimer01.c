// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	03/2001 - Written by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Check that a correct call to getitimer() succeeds.
 */

#include "tst_test.h"

static int itimer_name[] = {
	ITIMER_REAL,
	ITIMER_VIRTUAL,
	ITIMER_PROF,
};

static void run(void)
{
	long unsigned int i;
	struct itimerval value;

	for (i = 0; i < ARRAY_SIZE(itimer_name); i++) {
		TST_EXP_PASS(getitimer(itimer_name[i], &value));
		TST_EXP_EQ_LI(value.it_value.tv_sec, 0);
		TST_EXP_EQ_LI(value.it_value.tv_usec, 0);
	}
}

static struct tst_test test = {
	.test_all = run
};
