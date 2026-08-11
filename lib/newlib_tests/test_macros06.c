// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 zhanglianjie <zhanglianjie@uniontech.com>
 * Copyright (c) Linux Test Project, 2026
 */

/*
 * Test macros:
 *
 * - TST_EXP_VAL
 * - TST_EXP_VAL_SILENT
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

#define TEST_MACRO(macro, fail_fn, pass_fn, pass_val, fail_err) \
	do { \
		tst_res(TINFO, "* Testing " #macro "() macro"); \
		macro(fail_fn(), fail_err, #fail_fn"()"); \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(fail_fn(), fail_err); /* skip msg parameter */ \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(pass_fn(), pass_val, #pass_fn"()"); \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(fail_fn(), pass_val); /* skip msg parameter */ \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
	} while (0)

static void do_test(void)
{
	TEST_MACRO(TST_EXP_VAL, fail_val, pass_val, 42, 40);
	TEST_MACRO(TST_EXP_VAL_SILENT, fail_val, pass_val, 42, 40);
}

static struct tst_test test = {
	.test_all = do_test,
};
