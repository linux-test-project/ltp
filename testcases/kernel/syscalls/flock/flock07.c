// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 * Copyright (c) 2024 Linux Test Project
 */

/*\
 * [Description]
 *
 * Verify that flock(2) fails with errno EINTR when waiting to acquire a lock,
 * and the call is interrupted by a signal.
 */

#include <sys/file.h>
#include "tst_test.h"

#define TEMPFILE "test_eintr"

static int fd1 = -1, fd2 = -1;

static void handler(int sig)
{
	tst_res(TINFO, "Received signal: %s (%d)", tst_strsig(sig), sig);
}

static void setup(void)
{
	SAFE_TOUCH(TEMPFILE, 0777, NULL);
	fd1 = SAFE_OPEN(TEMPFILE, O_RDWR);
	fd2 = SAFE_OPEN(TEMPFILE, O_RDWR);
}

static void cleanup(void)
{
	if (fd1 >= 0)
		SAFE_CLOSE(fd1);

	if (fd2 >= 0)
		SAFE_CLOSE(fd2);
}

static void child_do(int fd)
{
	struct sigaction sa = {};

	sa.sa_handler = handler;
	SAFE_SIGEMPTYSET(&sa.sa_mask);
	SAFE_SIGACTION(SIGUSR1, &sa, NULL);

	tst_res(TINFO, "Trying to acquire exclusive lock from child");
	TST_EXP_FAIL(flock(fd, LOCK_EX), EINTR);
}

static void verify_flock(void)
{
	pid_t pid;

	TST_EXP_PASS(flock(fd1, LOCK_EX));

	pid = SAFE_FORK();
	if (!pid) {
		child_do(fd2);
		exit(0);
	} else {
		sleep(1);
		SAFE_KILL(pid, SIGUSR1);
		SAFE_WAITPID(pid, NULL, 0);
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_flock,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
};
