// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * Test for EFAULT when rlim points outside the accessible address space.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "tst_test.h"

static void *bad_addr;

static void verify_setrlimit(void)
{
	int status;
	pid_t pid;

	pid = SAFE_FORK();
	if (!pid) {
		TEST(setrlimit(RLIMIT_NOFILE, bad_addr));
		if (TST_RET != -1) {
			tst_res(TFAIL, "setrlimit()  succeeded unexpectedly");
			exit(0);
		}

		/* Usually, setrlimit() should return EFAULT */
		if (TST_ERR == EFAULT) {
			tst_res(TPASS | TTERRNO,
				"setrlimit() failed as expected");
		} else {
			tst_res(TFAIL | TTERRNO,
				"setrlimit() should fail with EFAULT, got");
		}

		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	/* If glibc has to convert between 32bit and 64bit struct rlimit
	 * in some cases, it is possible to get SegFault.
	 */
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "setrlimit() caused SIGSEGV");
		return;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return;

	tst_res(TFAIL, "child %s", tst_strstatus(status));
}

static void setup(void)
{
	bad_addr = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.test_all = verify_setrlimit,
	.forks_child = 1,
	.setup = setup,
};
