// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify basic setpgid() functionality, re-setting group ID inside both parent
 * and child. In the first case, we obtain getpgrp() and set it. In the second
 * case, we use setpgid(0, 0).
 */

#include "tst_test.h"

static void setpgid_test1(void)
{
	pid_t pgid, pid;

	pgid = TST_EXP_PID(getpgrp());
	pid = TST_EXP_PID(getpid());

	TST_EXP_PASS(setpgid(pid, pgid));
	TST_EXP_EQ_LI(pgid, getpgrp());
}

static void setpgid_test2(void)
{
	pid_t pgid;

	if (!SAFE_FORK()) {
		pgid = TST_EXP_PID(getpid());
		TST_EXP_PASS(setpgid(0, 0));
		TST_EXP_EQ_LI(pgid, getpgrp());
	}
}

static void run(void)
{
	setpgid_test1();
	setpgid_test2();
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
