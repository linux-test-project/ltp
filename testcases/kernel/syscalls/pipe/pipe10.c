// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, when a parent process opens a pipe, a child process can
 * read from it.
 */

#include <stdio.h>
#include "tst_test.h"

static int fds[2];

static void run(void)
{
	int wr_cnt, rd_cnt;
	char wrbuf[] = "abcdefghijklmnopqrstuvwxyz";
	char rdbuf[BUFSIZ];

	SAFE_PIPE(fds);
	wr_cnt = SAFE_WRITE(SAFE_WRITE_ALL, fds[1], wrbuf, sizeof(wrbuf));

	if (!SAFE_FORK()) {
		rd_cnt = SAFE_READ(1, fds[0], rdbuf, wr_cnt);
		TST_EXP_EQ_LU(wr_cnt, rd_cnt);
	}

	tst_reap_children();
	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
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
	.forks_child = 1,
	.cleanup = cleanup
};
