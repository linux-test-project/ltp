// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2002
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
	SAFE_WRITE(SAFE_WRITE_ALL, fd[1], wrbuf, SIZE);

	TST_CHECKPOINT_WAIT(0);

	SAFE_WRITE(SAFE_WRITE_ALL, fd[1], wrbuf, SIZE);
	exit(0);
}

static void verify_pipe(void)
{
	int status;
	int sig = 0;
	pid_t pid;

	memset(wrbuf, 'a', SIZE);

	TEST(pipe(fd));
	if (TST_RET == -1) {
		tst_res(TFAIL|TTERRNO, "pipe() failed");
		return;
	}

	pid = SAFE_FORK();
	if (pid == 0) {
		do_child();
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
