// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2002-2022
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * [Description]
 *
 * Verify that write(2) fails with errno EAGAIN when attempt to write to fifo
 * opened in O_NONBLOCK mode.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "tst_test.h"

static char fifo[100];
static int rfd, wfd;
static long page_size;

static void verify_write(void)
{
	char wbuf[8 * page_size];

	TST_EXP_FAIL(write(wfd, wbuf, sizeof(wbuf)), EAGAIN);
}

static void setup(void)
{
	page_size = getpagesize();

	char wbuf[17 * page_size];

	sprintf(fifo, "%s.%d", fifo, getpid());

	SAFE_MKNOD(fifo, S_IFIFO | 0777, 0);

	rfd = SAFE_OPEN(fifo, O_RDONLY | O_NONBLOCK);
	wfd = SAFE_OPEN(fifo, O_WRONLY | O_NONBLOCK);

	SAFE_WRITE(SAFE_WRITE_ANY, wfd, wbuf, sizeof(wbuf));
}

static void cleanup(void)
{
	if (rfd > 0)
		SAFE_CLOSE(rfd);

	if (wfd > 0)
		SAFE_CLOSE(wfd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_write,
};
