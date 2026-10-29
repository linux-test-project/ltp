// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2026
 */

/*
 * Test macros:
 *
 * - TST_EXP_PASS_OR_FAIL
 * - TST_EXP_FD_OR_FAIL
 */

#include "tst_test.h"

#define ERR_ERRNO EINVAL

static int fail_fn(void)
{
	errno = ERR_ERRNO;
	return -1;
}

static int pass_fn(void)
{
	return 0;
}

static int pass_fd(void)
{
	return 42;
}

#define TEST_MACRO(macro, fail_fn, pass_fn, fail_err) \
	do { \
		tst_res(TINFO, "* Testing " #macro "() macro"); \
		macro(fail_fn(), fail_err, #fail_fn"()"); \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(fail_fn(), fail_err); /* skip msg parameter */ \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(pass_fn(), 0, #pass_fn"()"); \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
		macro(fail_fn(), 0); /* skip msg parameter */ \
		tst_res(TINFO, "TST_PASS = %i", TST_PASS); \
	} while (0)

static void do_test(void)
{
	TEST_MACRO(TST_EXP_PASS_OR_FAIL, fail_fn, pass_fn, ERR_ERRNO);
	TEST_MACRO(TST_EXP_FD_OR_FAIL, fail_fn, pass_fd, ERR_ERRNO);
}

static struct tst_test test = {
	.test_all = do_test,
};
