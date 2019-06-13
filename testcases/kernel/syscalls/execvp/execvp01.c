// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "tst_test.h"

static void verify_execvp(void)
{
	pid_t pid;
	char *const args[] = {"execvp01_child", "canary", NULL};

	pid = SAFE_FORK();
	if (pid == 0) {
		execvp("execvp01_child", args);
		tst_brk(TFAIL | TERRNO,
			"Failed to execute execvp01_child");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.test_all = verify_execvp,
};
