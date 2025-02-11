// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 *   Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that getsid(2) fails with ESRCH errno when there is no
 * process found with process ID pid.
 */

#include "tst_test.h"

static void run(void)
{
	pid_t unused_pid;
	unused_pid = tst_get_unused_pid();

	TST_EXP_FAIL(getsid(unused_pid), ESRCH);
}

static struct tst_test test = {
	.test_all = run
};
