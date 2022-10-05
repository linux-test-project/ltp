// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Testcase to check the basic functionality of dup2().
 *
 * - Attempt to dup2() on an open file descriptor.
 * - Attempt to dup2() on a close file descriptor.
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

static char filename0[40], filename1[40];
static int fd0 = -1, fd1 = -1;

static struct tcase {
	char *desc;
	int is_close;
} tcases[] = {
	{"Test duping over an open fd", 0},
	{"Test duping over a close fd", 1},
};

static void run(unsigned int i)
{
	int fd2, rval;
	char buf[40];

	struct tcase *tc = tcases + i;

	tst_res(TINFO, "%s", tc->desc);

	fd0 = SAFE_CREAT(filename0, 0666);
	SAFE_WRITE(SAFE_WRITE_ALL, fd0, filename0, strlen(filename0));
	SAFE_CLOSE(fd0);

	fd1 = SAFE_CREAT(filename1, 0666);
	SAFE_WRITE(SAFE_WRITE_ALL, fd1, filename1, strlen(filename1));

	fd0 = SAFE_OPEN(filename0, O_RDONLY);
	SAFE_FCNTL(fd0, F_SETFD, 1);

	if (tc->is_close) {
		/* SAFE_CLOSE() sets the fd to -1 avoid it here */
		rval = fd1;
		SAFE_CLOSE(rval);
	}

	TEST(dup2(fd0, fd1));
	fd2 = TST_RET;
	if (TST_RET == -1) {
		tst_res(TFAIL, "call failed unexpectedly");
		goto free;
	}
	if (fd1 != fd2) {
		tst_res(TFAIL, "file descriptors don't match");
		goto free;
	}

	memset(buf, 0, sizeof(buf));
	SAFE_READ(0, fd2, buf, sizeof(buf));
	if (strcmp(buf, filename0) != 0)
		tst_res(TFAIL, "read from file got bad data");
	else
		tst_res(TPASS, "test the content of file is correct");

	rval = SAFE_FCNTL(fd2, F_GETFD);
	if (rval != 0)
		tst_res(TFAIL, "the FD_CLOEXEC flag is %#x, expected 0x0",
			rval);
	else
		tst_res(TPASS, "test the FD_CLOEXEC flag is correct");
free:
	SAFE_CLOSE(fd0);
	SAFE_CLOSE(fd1);
	SAFE_UNLINK(filename0);
	SAFE_UNLINK(filename1);
}

static void setup(void)
{
	int pid;

	pid = getpid();
	sprintf(filename0, "dup203.file0.%d\n", pid);
	sprintf(filename1, "dup203.file1.%d\n", pid);
}

static void cleanup(void)
{
	close(fd0);
	close(fd1);
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.test = run,
	.cleanup = cleanup,
};
