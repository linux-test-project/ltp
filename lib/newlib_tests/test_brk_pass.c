// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test that tst_brk(TPASS, ...) works properly.
 */
#include "tst_test.h"

static void do_test(void)
{
	int pid = SAFE_FORK();

	if (pid)
		return;

	tst_brk(TPASS, "Test child exiting...");
	tst_res(TWARN, "Test child stil alive!");
}

static struct tst_test test = {
	.test_all = do_test,
	.forks_child = 1,
};
