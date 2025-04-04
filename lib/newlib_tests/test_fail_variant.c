// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test that tst_brk(TFAIL, ...) exits only single test variant.
 */
#include "tst_test.h"

static void do_test(void)
{
	tst_brk(TFAIL, "Failing a test variant");
	tst_res(TWARN, "Shouldn't be reached");
}

static struct tst_test test = {
	.test_all = do_test,
	.test_variants = 10,
};
