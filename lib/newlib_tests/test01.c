// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * The test should abort when oldlib function is called from newlib.
 */

#include "tst_test.h"

void tst_exit(void);

static void do_test(void)
{
	tst_exit();
}

static struct tst_test test = {
	.test_all = do_test,
};
