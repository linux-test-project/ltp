// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 * History
 *	07/2001 John George
 *		-Ported
 */

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static void run(void)
{
	pid_t pid, pid1;

	pid = SAFE_FORK();
	if (pid > 0) {
		waitpid(pid, NULL, 0);
	} else {
		pid1 = setsid();
		if (pid1 < 0)
			tst_brk(TBROK | TTERRNO, "setsid failed");
		TEST(tst_syscall(__NR_vhangup));
		if (TST_RET == -1)
			tst_res(TFAIL | TTERRNO, "vhangup() failed");
		else
			tst_res(TPASS, "vhangup() succeeded");
	}
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_root = 1,
};
