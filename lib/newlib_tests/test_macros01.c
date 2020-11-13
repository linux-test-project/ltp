// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Test macros.
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

static void do_test(void)
{
	TST_EXP_FD(fail_fd(), "TEST DESCRIPTION");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FD(pass_fd(), "%s", "TEST DESCRIPTION PARAM");
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
	TST_EXP_FD(inval_val());
	tst_res(TINFO, "TST_PASS = %i", TST_PASS);
}

static struct tst_test test = {
	.test_all = do_test,
};
