// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test for result propagation.
 */

#include "tst_test.h"

static void setup(void)
{
	tst_res(TINFO, "setup() executed by pid %i", getpid());
}

static void cleanup(void)
{
	tst_res(TINFO, "cleanup() executed by pid %i", getpid());
}

static void do_test(void)
{
	unsigned int i;

	for (i = 0; i < 100; i++) {
		if (SAFE_FORK() == 0) {
			tst_res(TPASS, "Child (%i)", getpid());
			return;
		}
	}
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
