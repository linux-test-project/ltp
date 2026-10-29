// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * Tests basic error handling of the pidfd_open syscall.
 *
 * - EBADF pidfd is not a valid PID file descriptor
 * - EBADF targetfd is not an open file descriptor in the process referred
 *   to by pidfd
 * - EINVAL flags is not 0
 * - ESRCH the process referred to by pidfd does not exist (it has terminated
 *   and been waited on)
 * - EPERM the calling process doesn't have PTRACE_MODE_ATTACH_REALCREDS permissions
 *   over the process referred to by pidfd
 */

#include <stdlib.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/pidfd.h"

static int valid_pidfd = -1, invalid_pidfd = -1, pidfd = -1;
static uid_t uid;

static struct tcase {
	char *name;
	int *pidfd;
	int targetfd;
	int flags;
	int exp_errno;
} tcases[] = {
	{"invalid pidfd", &invalid_pidfd, 0, 0, EBADF},
	{"invalid targetfd", &valid_pidfd, -1, 0, EBADF},
	{"invalid flags", &valid_pidfd, 0, 1, EINVAL},
	{"the process referred to by pidfd doesn't exist", NULL, 0, 0, ESRCH},
	{"lack of required permission", &valid_pidfd, 0, 0, EPERM},
};

static void setup(void)
{
	pidfd_open_supported();
	pidfd_getfd_supported();

	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	uid = pw->pw_uid;

	valid_pidfd = SAFE_PIDFD_OPEN(getpid(), 0);
}

static void cleanup(void)
{
	if (valid_pidfd > -1)
		SAFE_CLOSE(valid_pidfd);
	if (pidfd > -1)
		SAFE_CLOSE(pidfd);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int pid;

	if (tc->exp_errno == EPERM) {
		pid = SAFE_FORK();
		if (!pid) {
			SAFE_SETUID(uid);
			TST_EXP_FAIL2(pidfd_getfd(valid_pidfd, tc->targetfd, tc->flags),
				tc->exp_errno, "pidfd_getfd(%d, %d, %d) with %s",
				valid_pidfd, tc->targetfd, tc->flags, tc->name);
			TST_CHECKPOINT_WAKE(0);
			exit(0);
		}
		TST_CHECKPOINT_WAIT(0);
		SAFE_WAIT(NULL);
		return;
	} else if (tc->exp_errno == ESRCH) {
		pid = SAFE_FORK();
		if (!pid) {
			TST_CHECKPOINT_WAIT(0);
			exit(0);
		}
		pidfd = SAFE_PIDFD_OPEN(pid, 0);
		TST_CHECKPOINT_WAKE(0);
		SAFE_WAIT(NULL);
		TST_EXP_FAIL2(pidfd_getfd(pidfd, tc->targetfd, tc->flags),
			tc->exp_errno, "pidfd_getfd(%d, %d, %d) with %s",
			pidfd, tc->targetfd, tc->flags, tc->name);
		SAFE_CLOSE(pidfd);
	} else	{
		TST_EXP_FAIL2(pidfd_getfd(*tc->pidfd, tc->targetfd, tc->flags),
			tc->exp_errno, "pidfd_getfd(%d, %d, %d) with %s",
			*tc->pidfd, tc->targetfd, tc->flags, tc->name);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
