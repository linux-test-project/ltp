// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * Copyright (c) Linux Test Project, 2002-2015
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, pipe(2) syscall fails with errno EMFILE when
 * limit on the number of open file descriptors has been reached.
 */

#include "tst_test.h"
#include <stdlib.h>

static int fds[2];
static int *opened_fds, num_opened_fds;

static void setup(void)
{
	int max_fds;

	max_fds = getdtablesize();
	tst_res(TINFO, "getdtablesize() = %d", max_fds);
	opened_fds = SAFE_MALLOC(max_fds * sizeof(int));

	do {
		SAFE_PIPE(fds);
		opened_fds[num_opened_fds++] = fds[0];
		opened_fds[num_opened_fds++] = fds[1];
	} while (fds[1] < max_fds - 2);

	tst_res(TINFO, "Number of fds opened by pipe calls: %d", num_opened_fds);
}

static void run(void)
{
	TST_EXP_FAIL(pipe(fds), EMFILE);
}

static void cleanup(void)
{
	for (int i = 0; i < num_opened_fds; i++)
		SAFE_CLOSE(opened_fds[i]);

	if (opened_fds)
		free(opened_fds);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run
};
