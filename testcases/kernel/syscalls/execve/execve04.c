/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) International Business Machines  Corp., 2001
 *
 *  07/2001 Ported by Wayne Boyer
 *  04/2008 Roy Lee <roylee@andestech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Attempt to execve(2) a file which is being opened by another process for
 * writing fails with ETXTBSY.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
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

static const char *const resource_files[] = {
	TEST_APP,
	NULL,
};

static struct tst_test test = {
	.test_all = verify_execve,
	.forks_child = 1,
	.child_needs_reinit = 1,
	.needs_checkpoints = 1,
	.resource_files = resource_files,
};
