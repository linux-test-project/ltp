// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test TST_EXP_PASS and TST_EXP_PASS_SILENT macro.
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
	tst_res(TINFO, "Testing TST_EXP_PASS macro");
	TST_EXP_PASS(fail_fn(), "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PASS(pass_fn(), "pass_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PASS(inval_ret_fn(), "inval_ret_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);

	tst_res(TINFO, "Testing TST_EXP_PASS_SILENT macro");
	TST_EXP_PASS_SILENT(fail_fn(), "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PASS_SILENT(pass_fn(), "pass_fn()");
	tst_res(TINFO, "TST_PASS = %i from TST_EXP_PASS_SILENT(pass_fn, ...)", TST_PASS);
	TST_EXP_PASS_SILENT(inval_ret_fn(), "inval_ret_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
}

static struct tst_test test = {
	.test_all = do_test,
};
