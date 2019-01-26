// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 John George
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Check the return value, and errno of vhangup(2) when a non-root user calls
 * vhangup().
 */
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

static uid_t nobody_uid;

static void run(void)
{
	pid_t pid;
	int retval;

	pid = SAFE_FORK();
	if (pid > 0) {
		waitpid(pid, NULL, 0);
	} else {
		retval = setreuid(nobody_uid, nobody_uid);
		if (retval < 0)
			tst_brk(TBROK | TTERRNO, "setreuid failed");
		TEST(tst_syscall(__NR_vhangup));
		if (TST_RET != -1)
			tst_brk(TFAIL, "vhangup() failed to fail");
		else if (TST_ERR == EPERM)
			tst_res(TPASS, "Got EPERM as expected.");
		else
			tst_res(TFAIL, "expected EPERM got %d", TST_ERR);
	}
}

static void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = pw->pw_uid;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.forks_child = 1,
};
