// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Test: checkpoint wait with timeout.
 * Expected: child blocks on checkpoint wait, parent exits without signaling.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "tst_checkpoint.h"

static void run(void)
{
	pid_t pid;

	pid = SAFE_FORK();

	if (pid == 0) {
		int ret = tst_checkpoint_wait(0, 1000);

		if (ret == -1 && errno == ETIMEDOUT)
			tst_res(TPASS, "Child: checkpoint wait timed out as expected");
		else
			tst_brk(TBROK | TERRNO, "checkpoint wait failed");

		_exit(0);
	}

	tst_res(TINFO, "Parent: exiting without signaling checkpoint");
	SAFE_WAITPID(pid, NULL, 0);

	return;
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
