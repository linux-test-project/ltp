// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 zhanglianjie <zhanglianjie@uniontech.com>
 */

/*
 * Test TST_EXP_VAL and TST_EXP_VAL_SILENT macro.
 */

#include "tst_test.h"

static int fail_val(void)
{
	errno = EINVAL;
	return 42;
}

static int pass_val(void)
{
	return 42;
}

static void do_test(void)
{
	tst_res(TINFO, "Testing TST_EXP_VAL macro");
	TST_EXP_VAL(fail_val(), 40, "fail_val()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_VAL(pass_val(), 42, "pass_val()");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);

	tst_res(TINFO, "Testing TST_EXP_VAL_SILENT macro");
	TST_EXP_VAL_SILENT(fail_val(), 40, "fail_val()");
	tst_res(TINFO, "TST_PASS = %i from TST_EXP_VAL_SILENT(fail_val, ...)", TST_PASS);
	TST_EXP_VAL_SILENT(pass_val(), 42, "pass_val()");
	tst_res(TINFO, "TST_PASS = %i from TST_EXP_VAL_SILENT(pass_val, ...)", TST_PASS);
}

static struct tst_test test = {
	.test_all = do_test,
};
