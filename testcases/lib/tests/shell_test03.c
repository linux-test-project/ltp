// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Shell test example.
 */

#include <sys/wait.h>
#include "tst_test.h"

static void run_test(void)
{
	int pid, status;

	pid = tst_run_script("shell_test_pass.sh", NULL);

	tst_res(TINFO, "Waiting for the pid %i", pid);

	waitpid(pid, &status, 0);

	tst_res(TINFO, "Shell test has %s", tst_strstatus(status));
}

static struct tst_test test = {
	.runs_script = 1,
	.test_all = run_test,
};
