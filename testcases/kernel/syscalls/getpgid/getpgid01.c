// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify the basic functionality of getpgid(2) syscall.
 */

#include "tst_test.h"

static void run(void)
{
	pid_t pid_1, child_pid, pgid;

	pgid = getpgid(0);
	tst_res(TINFO, "getpgid(0) in parent = %d", pgid);

	pid_1 = SAFE_FORK();
	if (!pid_1) {
		child_pid = getpid();

		tst_res(TINFO, "getpid() in child = %d", child_pid);
		tst_res(TINFO, "Running getpgid() in child");

		TST_EXP_PID(getpgid(0));
		TST_EXP_EQ_LI(TST_RET, pgid);

		TST_EXP_PID(getpgid(child_pid), "getpgid(%d)", child_pid);
		TST_EXP_EQ_LI(TST_RET, pgid);

		TST_EXP_PID(getpgid(pgid), "getpgid(%d)", pgid);
		TST_EXP_EQ_LI(TST_RET, pgid);

		TST_EXP_PID(getpgid(1));
		TST_EXP_EQ_LI(TST_RET, 1);
	}

	tst_reap_children();
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1
};
