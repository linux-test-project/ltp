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
	pid_t pid;

	pid = setsid();
	if (pid < 0)
		tst_res(TWARN | TTERRNO,
			"current process is already a group leader");
	TEST(tst_syscall(__NR_vhangup));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "vhangup() failed");
	else
		tst_res(TPASS, "vhangup() succeeded");
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
};
