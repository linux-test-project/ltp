// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Tests various corner conditions:
 *
 * - default message, i.e. first argument stringification
 * - macro indirection, i.e. we have to stringify early
 *
 * The output should include the MACRO_FAIL() as the either fail of pass
 * message. If it's missing or if it has been replaced by the function name
 * there is a bug in the TST_EXP_*() macro.
 */

#include "tst_test.h"

static int fail_fn_should_not_be_seen_in_output(void)
{
	errno = EINVAL;
	return -1;
}

#define MACRO_FAIL() fail_fn_should_not_be_seen_in_output()

static void do_test(void)
{
	TST_EXP_FAIL(MACRO_FAIL(), EINVAL);
	TST_EXP_FAIL2(MACRO_FAIL(), EINVAL);

	TST_EXP_PASS(MACRO_FAIL());
	TST_EXP_PASS_SILENT(MACRO_FAIL());

	TST_EXP_PID(MACRO_FAIL());
	TST_EXP_PID_SILENT(MACRO_FAIL());

	TST_EXP_FD(MACRO_FAIL());
	TST_EXP_FD_SILENT(MACRO_FAIL());

	TST_EXP_POSITIVE(MACRO_FAIL());
}

static struct tst_test test = {
	.test_all = do_test,
};
