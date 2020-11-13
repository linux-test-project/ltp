// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2020 SUSE LLC
 *
 */
/*\
 * [DESCRIPTION]
 * Negative test for dup(2) (too many fds).
 *
 * [ALGORITHM]
 * Open the maximum allowed number of file descriptors and then try to call
 * dup() once more and verify it fails with EMFILE.
\*/

#include <stdlib.h>
#include "tst_test.h"

int *fd;
int nfds;

static void run(void)
{
	TEST(dup(fd[0]));

	if (TST_RET < -1) {
		tst_res(TFAIL, "Invalid dup() return value %ld", TST_RET);
		return;
	}

	if (TST_RET == -1) {
		if (TST_ERR == EMFILE)
			tst_res(TPASS | TERRNO, "dup() failed as expected");
		else
			tst_res(TFAIL | TERRNO, "dup() failed unexpectedly");
		return;
	}

	tst_res(TFAIL, "dup() succeeded unexpectedly");
	SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	long maxfds;

	maxfds = sysconf(_SC_OPEN_MAX);
	if (maxfds == -1)
		tst_brk(TBROK, "sysconf(_SC_OPEN_MAX) failed");

	fd = SAFE_MALLOC(maxfds * sizeof(int));

	fd[0] = SAFE_OPEN("dupfile", O_RDWR | O_CREAT, 0700);

	for (nfds = 1; nfds < maxfds; nfds++) {
		fd[nfds] = SAFE_DUP(fd[0]);
		if (fd[nfds] >= maxfds - 1)
			break;
	}
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < nfds; i++)
		SAFE_CLOSE(fd[i]);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
