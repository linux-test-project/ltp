// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Test that we get an error using illegal signals.
 */

#include "tst_test.h"

static int sigs[] = { _NSIG + 1, SIGKILL, SIGSTOP };

static void do_test(unsigned int n)
{
	TST_EXP_FAIL2((long)signal(sigs[n], SIG_IGN), EINVAL);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(sigs),
	.test = do_test,
};
