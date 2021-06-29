// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test TST_EXP_FD and TST_EXP_FD_SILENT macro.
 */

#include "tst_test.h"

static int fail_fd(void)
{
	errno = EINVAL;
	return -1;
}

static int pass_fd(void)
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
	tst_res(TINFO, "Testing TST_EXP_FD macro");
	TST_EXP_FD(fail_fd(), "fail_fd()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FD(pass_fd(), "pass_fd()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FD(inval_val(), "inval_val()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FD(zero_val(), "zero_val()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);

	tst_res(TINFO, "Testing TST_EXP_FD_SILENT macro");
	TST_EXP_FD_SILENT(fail_fd(), "fail_fd()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FD_SILENT(pass_fd(), "%s", "pass_fd()");
	tst_res(TINFO, "TST_PASS = %i from TST_EXP_FD_SILENT(pass_fd, ...)", TST_PASS);
	TST_EXP_FD_SILENT(inval_val(), "inval_val()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FD_SILENT(zero_val(), "zero_val()");
	tst_res(TINFO, "TST_PASS = %i from TST_EXP_FD_SILENT(zero_val, ...)", TST_PASS);
}

static struct tst_test test = {
	.test_all = do_test,
};
