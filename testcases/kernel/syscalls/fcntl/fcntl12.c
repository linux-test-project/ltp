// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Tests basic error handling of the fcntl syscall.
 *
 * - EMFILE when cmd is F_DUPFD and the per-process limit on the number of open
 *   file descriptors has been reached.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include "tst_test.h"

static char fname[20] = "testfile";
static int fd = -1, max_files;

static void verify_fcntl(void)
{
	pid_t pid;
	int i;

	pid = SAFE_FORK();
	if (pid == 0) {
		for (i = 0; i < max_files; i++) {
			fd = open(fname, O_CREAT | O_RDONLY, 0444);
			if (fd == -1)
				break;
		}
		TST_EXP_FAIL2(fcntl(1, F_DUPFD, 1), EMFILE,
			"fcntl(1, F_DUPFD, 1)");
	}

	tst_reap_children();
}

static void setup(void)
{
	max_files = getdtablesize();
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);

	SAFE_UNLINK(fname);
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_fcntl,
};
