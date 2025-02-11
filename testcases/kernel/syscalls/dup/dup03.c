// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2020 SUSE LLC
 *
 */
/*\
 * Verify that dup(2) syscall fails with errno EMFILE when the per-process
 * limit on the number of open file descriptors has been reached.
 */

#include <stdlib.h>
#include "tst_test.h"

static int *fd;
static int nfds;

static void run(void)
{
	TST_EXP_FAIL2(dup(fd[0]), EMFILE, "dup(%d)", fd[0]);

	if (TST_RET != -1)
		SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	long maxfds;

	maxfds = SAFE_SYSCONF(_SC_OPEN_MAX);
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
