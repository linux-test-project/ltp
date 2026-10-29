// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test that tst_brk(TFAIL, ...) exits only single test variant.
 */
#include "tst_test.h"

static void do_test(void)
{
	int i;

	for (i = 0; i < 10; i++) {
		if (!SAFE_FORK()) {
			tst_res(TINFO, "Suspending child %i", i);
			pause();
		}
	}

	if (!SAFE_FORK())
		tst_brk(TBROK, "Child triggers TBROK");

	pause();
}

static struct tst_test test = {
	.test_all = do_test,
	.forks_child = 1,
};
