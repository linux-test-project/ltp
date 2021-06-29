// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*
 * Test TST_EXP_PID and TST_EXP_PID_SILENT macro.
 */

#include "tst_test.h"

static int fail_pid(void)
{
	errno = EINVAL;
	return -1;
}

static int pass_pid(void)
{
	return 42;
}

static int inval_val(void)
{
	return -42;
}

static int zero_val(void)
{
	return 0;
}

static void do_test(void)
{
	tst_res(TINFO, "Testing TST_EXP_PID macro");
	TST_EXP_PID(fail_pid(), "fail_pid()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PID(pass_pid(), "pass_pid()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PID(inval_val(), "inval_val()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PID(zero_val(), "zero_val()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);

	tst_res(TINFO, "Testing TST_EXP_PID_SILENT macro");
	TST_EXP_PID_SILENT(fail_pid(), "fail_pid()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PID_SILENT(pass_pid(), "%s", "pass_pid()");
	tst_res(TINFO, "TST_PASS = %i from TST_EXP_PID_SILENT(pass_pid, ...)", TST_PASS);
	TST_EXP_PID_SILENT(inval_val(), "inval_val()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_PID_SILENT(zero_val(), "zero_val()");
	tst_res(TINFO, "TST_PASS = %i from TST_EXP_PID_SILENT(zero_val, ...)", TST_PASS);
}

static struct tst_test test = {
	.test_all = do_test,
};
