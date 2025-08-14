// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 * Copyright (c) International Business Machines Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * Verify that setpgid(2) syscall fails with:
 *
 * - EINVAL when given pgid is less than 0.
 * - ESRCH when pid is not the calling process and not a child of
 *   the calling process.
 * - EPERM when an attempt was made to move a process into a nonexisting
 *   process group.
 */

#include <errno.h>
#include <unistd.h>
#include "tst_test.h"

static pid_t pgid, pid, ppid, inval_pgid;
static pid_t negative_pid = -1;

static struct tcase {
	pid_t *pid;
	pid_t *pgid;
	int error;
} tcases[] = {
	{&pid, &negative_pid, EINVAL},
	{&ppid, &pgid, ESRCH},
	{&pid, &inval_pgid, EPERM}
};

static void setup(void)
{
	pid = getpid();
	ppid = getppid();
	pgid = getpgrp();

	/*
	 * pid_max would not be in use by another process and guarantees that
	 * it corresponds to an invalid PGID, generating EPERM.
	 */
	SAFE_FILE_SCANF("/proc/sys/kernel/pid_max", "%d\n", &inval_pgid);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(setpgid(*tc->pid, *tc->pgid), tc->error,
				"setpgid(%d, %d)", *tc->pid, *tc->pgid);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases)
};
