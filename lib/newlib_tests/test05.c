// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test that child process that returns to test library code is reported.
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
	pid_t pid = SAFE_FORK();

	switch (pid) {
	case 0:
		tst_res(TPASS, "Child (%i) reports", getpid());
	break;
	default:
		tst_res(TPASS, "Parent (%i) reports", getpid());
	break;
	}
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
