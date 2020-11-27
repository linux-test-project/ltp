// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test that TBROK is propagated correctly to the results even if we wait on
 * child and throw away the status.
 */
#include "tst_test.h"

static void do_test(void)
{
	int pid = SAFE_FORK();

	if (pid) {
		tst_res(TPASS, "Test main pid");
		SAFE_WAITPID(pid, NULL, 0);
		return;
	}

	if (tst_variant == 1)
		tst_brk(TBROK, "Test child!");
	else
		tst_brk(TCONF, "Test child!");

	tst_res(TPASS, "Test child");
}

static struct tst_test test = {
	.test_all = do_test,
	.test_variants = 2,
	.forks_child = 1,
};
