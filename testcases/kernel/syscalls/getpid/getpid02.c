// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Check that:
 *
 * - fork() in parent returns the same pid as getpid() in child
 * - getppid() in child returns the same pid as getpid() in parent
 */

#include <errno.h>

#include "tst_test.h"

static pid_t *child_pid;

static void verify_getpid(void)
{
	pid_t proc_id;
	pid_t pid;
	pid_t pproc_id;

	proc_id = getpid();
	pid = SAFE_FORK();

	if (pid == 0) {
		pproc_id = getppid();

		if (pproc_id != proc_id) {
			tst_res(TFAIL, "child getppid() (%d) != parent getpid() (%d)",
				pproc_id, proc_id);
		} else {
			tst_res(TPASS, "child getppid() == parent getpid() (%d)", proc_id);
		}

		*child_pid = getpid();

		return;
	}

	tst_reap_children();

	if (*child_pid != pid)
		tst_res(TFAIL, "child getpid() (%d) != parent fork() (%d)", *child_pid, pid);
	else
		tst_res(TPASS, "child getpid() == parent fork() (%d)", pid);
}

static void setup(void)
{
	child_pid = SAFE_MMAP(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS | MAP_SHARED, -1, 0);
}

static void cleanup(void)
{
	SAFE_MUNMAP(child_pid, sizeof(pid_t));
}

static struct tst_test test = {
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_getpid,
};
