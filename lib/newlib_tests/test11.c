// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 */

/*
 * Test for segfault.
 */

#include "tst_test.h"

static void do_test(void)
{
	volatile char *ptr = NULL;

	*ptr = 0;

	tst_res(TPASS, "Not reached");
}

static struct tst_test test = {
	.test_all = do_test,
};
