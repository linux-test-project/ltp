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
static int flag_sighandle = 0;

void sighandle_sigpipe(int tmp LTP_ATTRIBUTE_UNUSED)
{
	flag_sighandle = 1;
	tst_res(TINFO, "SIGPIPE signal handler is called");
}

static void verify_pipe(void)
{

	memset(wrbuf, 'a', SIZE);
	SAFE_SIGNAL(SIGPIPE, sighandle_sigpipe);

	TEST(pipe(fd));
	if (TST_RET == -1) {
		tst_res(TFAIL|TERRNO, "pipe() failed");
		return;
	}

	memset(rdbuf, 0, SIZE);

	SAFE_WRITE(1, fd[1], wrbuf, SIZE);
	SAFE_READ(1, fd[0], rdbuf, SIZE);

	if (memcmp(wrbuf, rdbuf, SIZE) != 0) {
		tst_res(TFAIL, "pipe read data and pipe "
			"write data didn't match");
		return;
	}
	
	// close the pipe's reader file descriptor
	SAFE_CLOSE(fd[0]);

	// Test the broken pipe behaviour
	TEST(write(fd[1], wrbuf, SIZE));
	if (TST_RET == -1) {
		if (errno == EPIPE && flag_sighandle == 1) {
			tst_res(TPASS|TERRNO, "write returned as expected");
		} else {
			tst_res(TFAIL|TERRNO, "write failed with unexpected error");
		}
		
	} else {
		if (flag_sighandle == 1) {
			tst_res(TPASS, "write sucessed as expected");
		} else {
			tst_res(TFAIL, "write sucessed unexpectedly");
		}
	}
	SAFE_CLOSE(fd[1]);
}

static struct tst_test test = {
	.test_all = verify_pipe,
};
