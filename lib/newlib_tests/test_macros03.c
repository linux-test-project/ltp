// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test macros.
 */

#include "tst_test.h"

static int fail_fn(void)
{
	errno = EINVAL;
	return -1;
}

static int pass_fn(void)
{
	return 0;
}

static int inval_ret_fn(void)
{
	return 42;
}

static void do_test(void)
{
	TST_EXP_PASS(fail_fn());
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PASS(pass_fn(), "TEST DESCRIPTION");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PASS(inval_ret_fn());
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
}

static struct tst_test test = {
	.test_all = do_test,
};
