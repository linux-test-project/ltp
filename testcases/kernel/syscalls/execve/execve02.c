// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) International Business Machines Corp., 2001
 *
 * Ported to LTP: Wayne Boyer
 *  21/04/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 */

/*
 * Attempt to execve(2) an executable owned by root with no execute permissions
 * for the other users, fails when execve(2) is used as a non-root user, the
 * errno should be EACCES.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tst_test.h"

#define TEST_APP "execve_child"
#define USER_NAME "nobody"

static uid_t nobody_uid;

static void do_child(void)
{
	char *argv[2] = {TEST_APP, NULL};

	SAFE_SETEUID(nobody_uid);

	/* Use environ:
	 *     Inherit a copy of parent's environment
	 *     for tst_reinit() in execve_child.c
	 */
	TEST(execve(TEST_APP, argv, environ));

	if (!TST_RET)
		tst_brk(TFAIL, "execve() passed unexpectedly");

	if (TST_ERR != EACCES)
		tst_brk(TFAIL | TERRNO, "execve() failed unexpectedly");

	tst_res(TPASS | TERRNO, "execve() failed expectedly");

	exit(0);
}

static void verify_execve(void)
{
	pid_t pid = SAFE_FORK();

	if (pid == 0)
		do_child();
}

static void setup(void)
{
	struct passwd *pwd;

	SAFE_CHMOD(TEST_APP, 0700);

	pwd = SAFE_GETPWNAM(USER_NAME);
	nobody_uid = pwd->pw_uid;
}

static const char *const resource_files[] = {
	TEST_APP,
	NULL,
};

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.child_needs_reinit = 1,
	.setup = setup,
	.resource_files = resource_files,
	.test_all = verify_execve,
};
