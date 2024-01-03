// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test TST_EXP_FAIL and TST_EXP_FAIL2 macro.
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
	const int exp_errs_pass[] = {ENOTTY, EINVAL};
	const int exp_errs_fail[] = {ENOTTY, EISDIR};

	tst_res(TINFO, "Testing TST_EXP_FAIL macro");
	TST_EXP_FAIL(fail_fn(), EINVAL, "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL(fail_fn(), ENOTTY, "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL(pass_fn(), ENOTTY, "pass_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL(inval_ret_fn(), ENOTTY, "inval_ret_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL_ARR(fail_fn(), exp_errs_pass, "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL_ARR(fail_fn(), exp_errs_fail, "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);

	tst_res(TINFO, "Testing TST_EXP_FAIL2 macro");
	TST_EXP_FAIL2(fail_fn(), EINVAL, "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL2(fail_fn(), ENOTTY, "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL2(pass_fn(), ENOTTY, "pass_fn");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL2(inval_ret_fn(), ENOTTY, "inval_ret_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL2_ARR(fail_fn(), exp_errs_pass, "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FAIL2_ARR(fail_fn(), exp_errs_fail, "fail_fn()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
}

static struct tst_test test = {
	.test_all = do_test,
};
