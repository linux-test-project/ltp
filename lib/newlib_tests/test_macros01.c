// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2021 Yang Xu <xuyang2018.jy@fujitsu.com>
 * Copyright (c) Linux Test Project, 2026
 */

/*
 * Test macros:
 *
 * - TST_EXP_PASS
 * - TST_EXP_PASS_SILENT
 * - TST_EXP_FD
 * - TST_EXP_FD_SILENT
 * - TST_EXP_PID
 * - TST_EXP_PID_SILENT
 */

#include "tst_test.h"

static int fail_fn(void)
{
	errno = EINVAL;
	return -1;
}

static int pass_fn(void)
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

#define TEST_MACRO(macro, fail_fn, pass_fn, inval_fn, zero_val_fn) \
	do { \
		tst_res(TINFO, "* Testing " #macro "() macro"); \
		macro(fail_fn(), #fail_fn"()"); \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(fail_fn()); /* skip msg parameter */ \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(pass_fn(), #pass_fn"()"); \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(inval_fn(), #inval_fn"()"); \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(zero_val_fn(), #zero_val_fn"()"); \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
	} while (0)

static void do_test(void)
{
	TEST_MACRO(TST_EXP_PASS, fail_fn, pass_fn, inval_val, zero_val);
	TEST_MACRO(TST_EXP_PASS_SILENT, fail_fn, pass_fn, inval_val, zero_val);

	TEST_MACRO(TST_EXP_FD, fail_fn, pass_fn, inval_val, zero_val);
	TEST_MACRO(TST_EXP_FD_SILENT, fail_fn, pass_fn, inval_val, zero_val);

	TEST_MACRO(TST_EXP_PID, fail_fn, pass_fn, inval_val, zero_val);
	TEST_MACRO(TST_EXP_PID_SILENT, fail_fn, pass_fn, inval_val, zero_val);
}

static struct tst_test test = {
	.test_all = do_test,
};
