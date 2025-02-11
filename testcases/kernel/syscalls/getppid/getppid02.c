// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * Check that getppid() in child returns the same pid as getpid() in parent.
 */

#include <errno.h>

#include "tst_test.h"

static void verify_getppid(void)
{
	pid_t proc_id;
	pid_t pid;
	pid_t pproc_id;

	proc_id = getpid();
	pid = SAFE_FORK();
	if (pid == 0) {
		pproc_id = getppid();

		if (pproc_id != proc_id)
			tst_res(TFAIL, "child's ppid(%d) not equal to parent's pid(%d)",
				pproc_id, proc_id);
		else
			tst_res(TPASS, "getppid() returned parent pid (%d)", proc_id);
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.test_all = verify_getppid,
};
