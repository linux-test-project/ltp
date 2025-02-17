// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: Kathy Olmsted
 * Co-Pilot: Steve Shaw
 */

/*\
 * This test verifies that fork returns without error and that it returns the
 * pid of the child.
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tst_test.h"

#define	KIDEXIT	42
#define FILENAME "childpid"

static int fd = -1;

static void verify_fork(void)
{
	int kid_status, term_pid, child_pid, pid, ret;

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_FILE_PRINTF(FILENAME, "%d", getpid());
		exit(KIDEXIT);
	}

	term_pid = SAFE_WAITPID(pid, &kid_status, 0);
	if (term_pid == pid) {
		if (!WIFEXITED(kid_status)) {
			tst_res(TFAIL, "child exited abnormally");
			return;
		}
		ret = WEXITSTATUS(kid_status);
		if (ret != KIDEXIT)
			tst_res(TFAIL, "incorrect child status returned %d", ret);
		else
			tst_res(TPASS, "correct child status returned %d", ret);

		SAFE_FILE_SCANF(FILENAME, "%d", &child_pid);
		TST_EXP_EQ_LI(child_pid, pid);
	} else {
		tst_res(TFAIL, "waitpid() returns %d instead of expected pid %d",
				term_pid, pid);
	}

	tst_reap_children();
}

static void setup(void)
{
	fd = SAFE_CREAT(FILENAME, 0700);
	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd > -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.test_all = verify_fork,
};
