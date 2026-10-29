// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

#include "tst_test.h"

static void do_test(void)
{
	tst_res(TPASS, "Test passed!");
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_cpu_vendor = "Nonexistent",
};
