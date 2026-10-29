// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) International Business Machines Corp., 2001
 *
 * Ported to LTP: Wayne Boyer
 *  04/2008 Roy Lee <roylee@andestech.com>
 */

/*\
 * Attempt to :manpage:`execve(2)` a file which is being opened by another
 * process for writing fails with ETXTBSY.
 *
 * NOTE: write to executed file was allowed in 6.11-rc1:
 * 2a010c412853 ("fs: don't block i_writecount during exec")
 * but then reverted in v6.13-rc1:
 * 3b832035387f ("Revert "fs: don't block i_writecount during exec""),
 * backported into v6.11.11 and v6.12.2.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "tst_test.h"

#define TEST_APP "execve_child"

static void do_child(void);

static void verify_execve(void)
{
	pid_t pid;
	char *argv[2] = {TEST_APP, NULL};

	pid = SAFE_FORK();
	if (pid == 0)
		do_child();

	TST_CHECKPOINT_WAIT(0);

	TEST(execve(TEST_APP, argv, environ));

	if (TST_ERR != ETXTBSY)
		tst_res(TFAIL | TTERRNO, "execve succeeded, expected failure");
	else
		tst_res(TPASS | TTERRNO, "execve failed as expected");

	TST_CHECKPOINT_WAKE(0);
}

static void do_child(void)
{
	int fd = SAFE_OPEN(TEST_APP, O_WRONLY);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_CLOSE(fd);

	exit(0);
}

static struct tst_test test = {
	.test_all = verify_execve,
	.forks_child = 1,
	.child_needs_reinit = 1,
	.needs_checkpoints = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "3b832035387ff508fdcf0fba66701afc78f79e3d"},
		{}
	},
	.resource_files = (const char *const []) {
		TEST_APP,
		NULL
	}
};
