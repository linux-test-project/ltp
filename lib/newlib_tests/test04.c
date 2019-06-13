// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Tests tst_run_as_child()
 */

#include "tst_test.h"

static void child_fn(unsigned int i)
{
	switch (i) {
	case 0:
		tst_res(TPASS, "PASSED message");
	break;
	case 1:
		tst_brk(TBROK, "BROKEN message");
	break;
	}
}

static void setup(void)
{
	tst_res(TINFO, "setup() executed by pid %i", getpid());
}

static void cleanup(void)
{
	tst_res(TINFO, "cleanup() executed by pid %i", getpid());
}

static void do_test(unsigned int i)
{
	if (SAFE_FORK() == 0)
		child_fn(i);
}

static struct tst_test test = {
	.tcnt = 2,
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
