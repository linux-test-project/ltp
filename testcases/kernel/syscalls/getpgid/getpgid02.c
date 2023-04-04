// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that getpgid(2) fails with errno ESRCH when
 * pid does not match any process.
 */

#include "tst_test.h"

static pid_t unused_pid;
static pid_t neg_pid = -99;

static void setup(void)
{
	unused_pid = tst_get_unused_pid();
}

static void run(void)
{
	TST_EXP_FAIL2(getpgid(neg_pid), ESRCH, "getpgid(%d)", neg_pid);
	TST_EXP_FAIL2(getpgid(unused_pid), ESRCH, "getpgid(%d)", unused_pid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run
};
