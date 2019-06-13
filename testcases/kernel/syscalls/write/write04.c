// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *   Copyright (c) International Business Machines Corp., 2001
 */

/*
 * DESCRIPTION
 *	Testcase to check that write() sets errno to EAGAIN
 *
 * ALGORITHM
 *	Create a named pipe (fifo), open it in O_NONBLOCK mode, and
 *	attempt to write to it when it is full, write(2) should fail
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

	TEST(write(wfd, wbuf, sizeof(wbuf)));

	if (TST_RET != -1) {
		tst_res(TFAIL, "write() succeeded unexpectedly");
		return;
	}

	if (TST_ERR != EAGAIN) {
		tst_res(TFAIL | TTERRNO,
			"write() failed unexpectedly, expected EAGAIN");
		return;
	}

	tst_res(TPASS | TTERRNO, "write() failed expectedly");
}

static void setup(void)
{
	page_size = getpagesize();

	char wbuf[17 * page_size];

	sprintf(fifo, "%s.%d", fifo, getpid());

	SAFE_MKNOD(fifo, S_IFIFO | 0777, 0);

	rfd = SAFE_OPEN(fifo, O_RDONLY | O_NONBLOCK);
	wfd = SAFE_OPEN(fifo, O_WRONLY | O_NONBLOCK);

	SAFE_WRITE(0, wfd, wbuf, sizeof(wbuf));
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
