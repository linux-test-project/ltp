// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported from Crackerjack to LTP by Manas Kumar Nayak maknayak@in.ibm.com>
 */

/*\
 * [Description]
 *
 * Basic tests for the unshare(2) errors.
 *
 * - EINVAL on invalid flags
 * - EPERM when process is missing required privileges
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <sched.h>
#include <limits.h>
#include <unistd.h>
#include <pwd.h>

#include "tst_test.h"
#include "config.h"

#ifdef HAVE_UNSHARE

static uid_t nobody_uid;

static struct test_case_t {
	int mode;
	int expected_error;
	const char *desc;
} tc[] = {
	{-1, EINVAL, "-1"},
	{CLONE_NEWNS, EPERM, "CLONE_NEWNS"}
};

static void run(unsigned int i)
{
	pid_t pid = SAFE_FORK();
	if (pid == 0) {
		if (tc[i].expected_error == EPERM)
			SAFE_SETUID(nobody_uid);

		TST_EXP_FAIL(unshare(tc[i].mode), tc[i].expected_error,
			     "unshare(%s)", tc[i].desc);
	}
}

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");
	nobody_uid = ltpuser->pw_uid;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.forks_child = 1,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
	.test = run,
};

#else
TST_TEST_TCONF("unshare is undefined.");
#endif
