// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025 lufei <lufei@uniontech.com>
 */

/*\
 * This test case verifies unshare(CLONE_NEWPID) creates a new PID namespace
 * and that the first child process in the new namespace gets PID 1.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/sched.h"

static struct tst_clone_args *args;

static void setup(void)
{
	args->flags = CLONE_NEWPID;
	args->exit_signal = SIGCHLD;
}

static void run(void)
{
	if (SAFE_CLONE(args))
		return;

	SAFE_UNSHARE(CLONE_NEWPID);

	if (!SAFE_FORK()) {
		TST_EXP_EQ_LI(getpid(), 1);
		exit(0);
	}

	tst_reap_children();
}

static struct tst_test test = {
	.setup = setup,
	.forks_child = 1,
	.needs_root = 1,
	.test_all = run,
	.bufs = (struct tst_buffers []) {
		{&args, .size = sizeof(*args)},
		{},
	}
};
