// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2007
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Clone a process with CLONE_NEWPID flag and check:
 *
 * - child session ID must be 1
 * - parent process group ID must be 1
 */

#include "tst_test.h"
#include "lapi/sched.h"

static void child_func(void)
{
	TST_EXP_EQ_LI(getsid(0), 0);
	TST_EXP_EQ_LI(getpgid(0), 0);

	tst_res(TINFO, "setsid()");
	SAFE_SETSID();

	TST_EXP_EQ_LI(getsid(0), 1);
	TST_EXP_EQ_LI(getpgid(0), 1);
}

static void run(void)
{
	const struct tst_clone_args args = {
		.flags = CLONE_NEWPID,
		.exit_signal = SIGCHLD,
	};

	if (!SAFE_CLONE(&args)) {
		child_func();
		return;
	}
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_PID_NS",
		NULL,
	},
};
