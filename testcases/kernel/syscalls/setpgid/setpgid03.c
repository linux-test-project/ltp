// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Tests setpgid(2) errors:
 *
 * - EPERM The process specified by pid must not be a session leader.
 * - EPERM The calling process, process specified by pid and the target
 *   process group must be in the same session.
 * - EACCESS Proccess cannot change process group ID of a child after child
 *   has performed exec()
 */

#include <unistd.h>
#include <sys/wait.h>
#include "tst_test.h"

#define TEST_APP "setpgid03_child"

static void do_child(void)
{
	SAFE_SETSID();
	TST_CHECKPOINT_WAKE_AND_WAIT(0);
}

static void run(void)
{
	pid_t child_pid;

	child_pid = SAFE_FORK();
	if (!child_pid) {
		do_child();
		return;
	}

	TST_CHECKPOINT_WAIT(0);

	TST_EXP_FAIL(setpgid(child_pid, getppid()), EPERM);
	/* Child did setsid(), so its PGID is set to its PID. */
	TST_EXP_FAIL(setpgid(0, child_pid), EPERM);

	TST_CHECKPOINT_WAKE(0);

	/* child after exec() we are no longer allowed to set pgid */
	child_pid = SAFE_FORK();
	if (!child_pid)
		SAFE_EXECLP(TEST_APP, TEST_APP, NULL);

	TST_CHECKPOINT_WAIT(0);

	TST_EXP_FAIL(setpgid(child_pid, getppid()), EACCES);

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
