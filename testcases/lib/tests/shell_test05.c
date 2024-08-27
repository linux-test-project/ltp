// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Shell test example.
 */

#include "tst_test.h"

static void run_test(void)
{
	int pid;

	pid = tst_run_script("shell_test_checkpoint.sh", NULL);

	tst_res(TINFO, "Waiting for shell to sleep on checkpoint!");

	TST_PROCESS_STATE_WAIT(pid, 'S', 10000);

	tst_res(TINFO, "Waking shell child!");

	TST_CHECKPOINT_WAKE(0);
}

static struct tst_test test = {
	.runs_script = 1,
	.needs_checkpoints = 1,
	.test_all = run_test,
};
