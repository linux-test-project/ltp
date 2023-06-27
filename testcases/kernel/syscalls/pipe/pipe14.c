// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, if the write end of a pipe is closed, then a process reading
 * from the pipe will see end-of-file (i.e., read() returns 0) once it has
 * read all remaining data in the pipe.
 */

#include "tst_test.h"

static int fds[2];

static void run(void)
{
	char wrbuf[] = "abcdefghijklmnopqrstuvwxyz";
	char rdbuf[30];

	memset(rdbuf, 0, sizeof(rdbuf));
	SAFE_PIPE(fds);

	SAFE_WRITE(SAFE_WRITE_ALL, fds[1], wrbuf, sizeof(wrbuf));
	SAFE_CLOSE(fds[1]);

	SAFE_READ(0, fds[0], rdbuf, sizeof(wrbuf));

	TST_EXP_VAL(SAFE_READ(0, fds[0], rdbuf, 1), 0);
	SAFE_CLOSE(fds[0]);
}

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);
	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup
};
