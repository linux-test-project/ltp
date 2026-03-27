// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that :manpage:`open(2)` fails with EMFILE when per-process limit on the number
 * of open file descriptors has been reached.
 */

#include <stdio.h>
#include <stdlib.h>
#include "tst_test.h"

#define FNAME "open04"

static int fds_limit;
static int first = -1;
static int *fds;
static char fname[PATH_MAX];

static void setup(void)
{
	int fd;
	struct rlimit rlim;

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &rlim);
	fds_limit = rlim.rlim_cur;
	first = SAFE_OPEN(FNAME, O_RDWR | O_CREAT, 0777);

	fds = SAFE_MALLOC(sizeof(int) * (fds_limit - first));
	memset(fds, -1, sizeof(int) * (fds_limit - first));
	fds[0] = first;

	for (int i = first + 1; i < fds_limit; i++) {
		snprintf(fname, sizeof(fname), FNAME ".%d", i);
		fd = open(fname, O_RDWR | O_CREAT, 0777);
		if (fd == -1) {
			if (errno != EMFILE)
				tst_brk(TBROK, "Expected EMFILE but got %d", errno);
			fds_limit = i;
			break;
		}
		fds[i - first] = fd;
	}
}

static void run(void)
{
	snprintf(fname, sizeof(fname), FNAME ".%d", fds_limit);
	TST_EXP_FAIL2(open(fname, O_RDWR | O_CREAT, 0777), EMFILE);
}

static void cleanup(void)
{
	if (first >= 0 && fds) {
		int limit = fds_limit - first;

		for (int i = 0; i < limit; i++) {
			if (fds[i] != -1)
				SAFE_CLOSE(fds[i]);
		}
	}

	free(fds);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1
};
