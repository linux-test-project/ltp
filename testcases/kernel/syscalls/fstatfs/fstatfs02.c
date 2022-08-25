// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Testcase to check if fstatfs() sets errno correctly.
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

static struct statfs buf;

static int fd;
static int bad_fd = -1;

static struct test_case_t {
	int *fd;
	struct statfs *sbuf;
	int error;
} tests[] = {
	{&bad_fd, &buf, EBADF},
	{&fd, (void *)-1, EFAULT},
};

static void fstatfs_verify(unsigned int n)
{
	int pid, status;

	pid = SAFE_FORK();
	if (!pid) {
		TST_EXP_FAIL(fstatfs(*tests[n].fd, tests[n].sbuf), tests[n].error, "fstatfs()");
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return;

	if (tests[n].error == EFAULT &&
	    WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TPASS, "Got SIGSEGV instead of EFAULT");
		return;
	}

	tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static void setup(void)
{
	fd = SAFE_OPEN("tempfile", O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test = fstatfs_verify,
	.tcnt = ARRAY_SIZE(tests),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
