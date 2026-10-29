// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
 */
/*\
 * For a terminated child, test whether wait(2) can get its pid
 * and exit status correctly.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "tst_test.h"

static void verify_wait(void)
{
	int status, exit_child = 1;
	pid_t fpid;

	fpid = SAFE_FORK();
	if (fpid == 0)
		exit(exit_child);

	TST_EXP_PID_SILENT(wait(&status));

	if (!TST_PASS)
		return;

	if (fpid != TST_RET) {
		tst_res(TFAIL, "wait() returned pid %ld, expected %d",
			TST_RET, fpid);
		return;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == exit_child) {
		tst_res(TPASS, "wait() succeeded");
		return;
	}

	tst_res(TFAIL, "wait() reported child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = verify_wait,
	.forks_child = 1,
};
