// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2024 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Check that waitpid() returns the exit status of a specific child process
 * and repeated call on the same process will fail with ECHILD.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

#include "tst_test.h"

#define	MAX_CHILDREN 25

static pid_t children[MAX_CHILDREN];

static void check_waitpid(pid_t pid, int reaped)
{
	TEST(waitpid(pid, NULL, 0));

	if (!reaped && pid == (pid_t)TST_RET) {
		tst_res(TPASS, "waitpid(%d) returned correct PID", pid);
		return;
	}

	if (reaped && TST_RET == -1 && TST_ERR == ECHILD) {
		tst_res(TPASS | TTERRNO, "waitpid(%d) failed on reaped child",
			pid);
		return;
	}

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "waitpid(%d) failed", pid);
		return;
	}

	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO,
			"Unexpected waitpid(%d) return value %ld", pid,
			TST_RET);
		return;
	}

	tst_res(TFAIL, "waitpid(%d) returned unexpected PID %ld", pid, TST_RET);
}

static void run(void)
{
	int i;

	for (i = 0; i < MAX_CHILDREN; i++) {
		children[i] = SAFE_FORK();

		/* Children have nothing to do... */
		if (!children[i])
			exit(0);
	}

	/* Wait for one specific child */
	i = MAX_CHILDREN / 2;
	check_waitpid(children[i], 0);

	/* Try the same child again after it was reaped */
	check_waitpid(children[i], 1);
	tst_reap_children();
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1
};
