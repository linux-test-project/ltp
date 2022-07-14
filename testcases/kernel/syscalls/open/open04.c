// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that open(2) fails with EMFILE when per-process limit on the number
 * of open file descriptors has been reached.
 */

#include <stdio.h>
#include <stdlib.h>
#include "tst_test.h"

#define FNAME "open04"

static int fds_limit, first, i;
static int *fds;
static char fname[20];

static void setup(void)
{
	int fd;

	fds_limit = getdtablesize();
	first = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0777);

	fds = SAFE_MALLOC(sizeof(int) * (fds_limit - first));
	fds[0] = first;

	for (i = first + 1; i < fds_limit; i++) {
		sprintf(fname, FNAME ".%d", i);
		fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0777);
		fds[i - first] = fd;
	}
}

static void run(void)
{
	sprintf(fname, FNAME ".%d", fds_limit);
	TST_EXP_FAIL2(open(fname, O_RDWR | O_CREAT, 0777), EMFILE);
}

static void cleanup(void)
{
	if (!first || !fds)
		return;

	for (i = first; i < fds_limit; i++)
		SAFE_CLOSE(fds[i - first]);

	if (fds)
		free(fds);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1
};
