// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 */

/*
 * Test for timeout override.
 */

#include "tst_test.h"

static void do_test(void)
{
	sleep(1);
	tst_res(TPASS, "Passed!");
}

static struct tst_test test = {
	.timeout = 2,
	.test_all = do_test,
};
