// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test verifies error codes for :manpage:`process_mrelease(2)`:
 *
 * - EBADF when invalid file descriptor is passed
 * - EINVAL when flags parameter is not zero
 * - EINVAL when target process is still alive and not dying
 * - ESRCH when target process has terminated
 *
 * [Algorithm]
 *
 * - Call process_mrelease() with bad fd (-1) to expect EBADF
 * - Spawn child, call process_mrelease() with invalid flags (-1) to expect EINVAL
 * - Call process_mrelease() on running child without pending exit to expect EINVAL
 * - Wait for child exit, call process_mrelease() on dead child to expect ESRCH
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

static int badfd = -1;
static int pidfd;

enum {
	NO_CHILD,
	EXIT_CHILD,
	WAIT_CHILD,
};

static struct tcase {
	int child_type;
	int *fd;
	int flags;
	int exp_errno;
	char *msg;
} tcases[] = {
	{NO_CHILD, &badfd, 0, EBADF, "bad file descriptor"},
	{WAIT_CHILD, &pidfd, -1, EINVAL, "flags is not 0"},
	{WAIT_CHILD, &pidfd, 0, EINVAL, "task memory cannot be released"},
	{EXIT_CHILD, &pidfd, 0, ESRCH, "child is not running"},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int status;
	pid_t pid = -1;

	if (tc->child_type != NO_CHILD) {
		pid = SAFE_FORK();
		if (!pid) {
			if (tc->child_type == WAIT_CHILD)
				TST_CHECKPOINT_WAIT(0);

			exit(0);
		}

		tst_res(TINFO, "Spawned waiting child with pid=%d", pid);

		pidfd = SAFE_PIDFD_OPEN(pid, 0);

		if (tc->child_type == EXIT_CHILD)
			SAFE_WAITPID(pid, &status, 0);
	}

	TST_EXP_FAIL(tst_syscall(__NR_process_mrelease, *tc->fd, tc->flags),
		tc->exp_errno,
		"%s", tc->msg);

	if (tc->child_type != NO_CHILD) {
		if (tc->child_type == WAIT_CHILD) {
			TST_CHECKPOINT_WAKE(0);
			SAFE_WAITPID(pid, &status, 0);
		}

		SAFE_CLOSE(pidfd);
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.min_kver = "5.15",
	.needs_checkpoints = 1,
};
