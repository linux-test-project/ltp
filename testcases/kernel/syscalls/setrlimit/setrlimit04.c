// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Red Hat, Inc.  All rights reserved.
 */

/*\
 * Attempt to run a trivial binary with stack < 1MB.
 *
 * Early patches for stack guard gap caused that gap size was contributing to
 * stack limit. This caused failures for new processes (E2BIG) when ulimit was
 * set to anything lower than size of gap.
 *
 * Kernel commit 1be7107fbe18 ("mm: largerstack guard gap, between vmas")
 * from v4.12 sets default gap size to 1M (for systems with 4k pages), so let's
 * set stack limit to 512kB and confirm we can still run some trivial binary.
 */

#define _GNU_SOURCE
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "tst_test.h"

#define STACK_LIMIT (512 * 1024)

static void test_setrlimit(void)
{
	int status;
	struct rlimit rlim;
	pid_t child;

	rlim.rlim_cur = STACK_LIMIT;
	rlim.rlim_max = STACK_LIMIT;

	SAFE_SETRLIMIT(RLIMIT_STACK, &rlim);

	child = SAFE_FORK();
	if (child == 0)
		SAFE_EXECLP("/bin/true", "/bin/true", NULL);
	SAFE_WAITPID(child, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		tst_res(TPASS, "child process completed OK");
		return;
	}

	tst_res(TFAIL, "child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all     = test_setrlimit,
	.forks_child  = 1,
	.needs_root = 1,
};
