// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Testcase to check if read() successfully sets errno to EAGAIN when read from
 * a pipe (fifo, opened in O_NONBLOCK mode) without writing to it.
 */

#include <stdio.h>
#include <fcntl.h>
#include "tst_test.h"

static char fifo[100];
static int rfd, wfd;

static void verify_read(void)
{
	int c;

	TST_EXP_FAIL(read(rfd, &c, 1), EAGAIN,
		     "read() when nothing is written to a pipe");
}

static void setup(void)
{
	struct stat buf;

	sprintf(fifo, "fifo.%d", getpid());

	SAFE_MKNOD(fifo, S_IFIFO | 0777, 0);
	SAFE_STAT(fifo, &buf);

	if ((buf.st_mode & S_IFIFO) == 0)
		tst_brk(TBROK, "Mode does not indicate fifo file");

	rfd = SAFE_OPEN(fifo, O_RDONLY | O_NONBLOCK);
	wfd = SAFE_OPEN(fifo, O_WRONLY | O_NONBLOCK);
}

static void cleanup(void)
{
	SAFE_CLOSE(rfd);
	SAFE_CLOSE(wfd);
	SAFE_UNLINK(fifo);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_read,
};
