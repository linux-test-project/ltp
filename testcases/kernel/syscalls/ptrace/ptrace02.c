// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com
 *
 * ptrace() returns -1 and sets errno to EPERM if tracer doesn't have
 * CAP_SYS_PTRACE capability for the process. Such as nobody user.
 */

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include "tst_test.h"

uid_t uid;

static void verify_ptrace(void)
{
	int child_pid[2];

	child_pid[0] = SAFE_FORK();
	if (!child_pid[0])
		pause();

	child_pid[1] = SAFE_FORK();
	if (!child_pid[1]) {
		SAFE_SETUID(uid);
		TEST(ptrace(PTRACE_ATTACH, child_pid[0], NULL, NULL));

		if (TST_RET == 0) {
			tst_res(TFAIL, "ptrace() succeeded unexpectedly");
			exit(0);
		}

		if (TST_RET != -1) {
			tst_res(TFAIL,
				"Invalid ptrace() return value %ld", TST_RET);
			exit(0);
		}

		if (TST_ERR == EPERM)
			tst_res(TPASS | TTERRNO, "ptrace() failed as expected");
		else
			tst_res(TFAIL | TTERRNO, "ptrace() expected EPERM, but got");
		exit(0);
	}
	SAFE_WAITPID(child_pid[1], NULL, 0);
	SAFE_KILL(child_pid[0], SIGKILL);
	SAFE_WAITPID(child_pid[0], NULL, 0);
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	uid = pw->pw_uid;
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_ptrace,
	.forks_child = 1,
	.needs_root = 1,
};
