// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Parent process: spawns a child which reinitializes checkpoint region
 * using tst_reinit(). Waits for a checkpoint signal from the child.
 */

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "tst_checkpoint.h"

static void run(void)
{
	pid_t pid = SAFE_FORK();

	if (pid == 0) {
		TEST(execlp("tst_checkpoint_child", "tst_checkpoint_child", "canary", NULL));
		tst_brk(TFAIL | TTERRNO, "Failed to execute tst_checkpoint_child");
	}

	TST_CHECKPOINT_WAIT(0);
	tst_res(TPASS, "Parent: checkpoint reached");

	SAFE_WAITPID(pid, NULL, 0);

	return;
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_checkpoints = 1,
	.child_needs_reinit = 1,
	.test_all = run,
};
