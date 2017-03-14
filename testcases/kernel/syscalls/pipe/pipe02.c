/*
 * Copyright (c) International Business Machines  Corp., 2002
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 */

/*
 * Check that if a child has a "broken pipe", this information
 * is transmitted to the waiting parent.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

#define SIZE	5

static int fd[2];
static char rdbuf[SIZE];
static char wrbuf[SIZE];

static void do_child(void)
{
	SAFE_SIGNAL(SIGPIPE, SIG_DFL);
	SAFE_CLOSE(fd[0]);
	SAFE_WRITE(1, fd[1], wrbuf, SIZE);

	TST_CHECKPOINT_WAIT(0);

	SAFE_WRITE(1, fd[1], wrbuf, SIZE);
	exit(0);
}

static void verify_pipe(void)
{
	int status;
	int sig = 0;
	pid_t pid;

	memset(wrbuf, 'a', SIZE);

#ifdef UCLINUX
	maybe_run_child(&do_child, "dd", &fd[0], &fd[1]);
#endif

	TEST(pipe(fd));
	if (TEST_RETURN == -1) {
		tst_res(TFAIL|TERRNO, "pipe() failed");
		return;
	}

	pid = SAFE_FORK();
	if (pid == 0) {
#ifdef UCLINUX
		if (self_exec(av[0], "dd", fd[0], fd[1]) < 0)
			tst_brk(TBROK, "self_exec failed");
#else
		do_child();
#endif
	}

	memset(rdbuf, 0, SIZE);
	SAFE_CLOSE(fd[1]);
	SAFE_READ(1, fd[0], rdbuf, SIZE);

	if (memcmp(wrbuf, rdbuf, SIZE) != 0) {
		tst_res(TFAIL, "pipe read data and pipe "
			"write data didn't match");
		return;
	}

	SAFE_CLOSE(fd[0]);
	TST_CHECKPOINT_WAKE(0);
	SAFE_WAIT(&status);

	if (!WIFSIGNALED(status)) {
		tst_res(TFAIL, "Child wasn't killed by signal");
	} else {
		sig = WTERMSIG(status);
		if (sig != SIGPIPE) {
			tst_res(TFAIL, "Child killed by %s expected SIGPIPE",
				tst_strsig(sig));
		} else {
				tst_res(TPASS, "Child killed by SIGPIPE");
		}
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_checkpoints = 1,
	.test_all = verify_pipe,
};
