// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * tst_brkm() from SAFE_MACROS() is redirected to newlib
 */

#include "tst_test.h"

static void do_test(void)
{
	SAFE_CHDIR("nonexistent");
}

static struct tst_test test = {
	.test_all = do_test,
};
