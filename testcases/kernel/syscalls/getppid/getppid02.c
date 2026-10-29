// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * Check that:
 *
 * - :manpage:`fork(2)` in parent returns the same pid as
 *   :manpage:`getpid(2)` in child
 * - :manpage:`getppid(2)` in child returns the same pid as
 *   :manpage:`getpid(2)` in parent
 */

#include "tst_test.h"

static pid_t *child_pid;

static void verify_getppid(void)
{
	pid_t parent_pid;
	pid_t pid;

	parent_pid = getpid();
	pid = SAFE_FORK();

	if (pid == 0) {
		TST_EXP_EQ_LI(getppid(), parent_pid);

		*child_pid = getpid();
		return;
	}

	tst_reap_children();

	TST_EXP_EQ_LI(*child_pid, pid);
}

static void setup(void)
{
	child_pid = SAFE_MMAP(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE,
			      MAP_ANONYMOUS | MAP_SHARED, -1, 0);
}

static void cleanup(void)
{
	if (child_pid)
		SAFE_MUNMAP(child_pid, sizeof(pid_t));
}

static struct tst_test test = {
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_getppid,
};
