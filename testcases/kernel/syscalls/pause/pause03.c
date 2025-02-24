// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verifies that pause() does not return after proccess receives a SIGKILL signal.
 */

#include "tst_test.h"

void do_child(void)
{
	TST_CHECKPOINT_WAKE(0);
	pause();
	tst_res(TFAIL, "pause() returned after SIGKILL");
}

void run(void)
{
	int status;
	pid_t pid;

	pid = SAFE_FORK();
	if (!pid) {
		do_child();
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);
	TST_PROCESS_STATE_WAIT(pid, 'S', 10000);
	SAFE_KILL(pid, SIGKILL);
	SAFE_WAITPID(pid, &status, 0);

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL)
		tst_res(TPASS, "pause() killed by SIGKILL");
	else
		tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = run,
	.needs_checkpoints = 1,
	.forks_child = 1,
};
