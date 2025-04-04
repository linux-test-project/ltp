// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test that tst_brk(TBROK, ...) exits the test immediately.
 */
#include "tst_test.h"

static void do_test(void)
{
	tst_brk(TBROK, "Exitting the test during the first variant");
}

static struct tst_test test = {
	.test_all = do_test,
	.test_variants = 10,
};
