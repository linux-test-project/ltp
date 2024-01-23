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
#include <stdlib.h>

#include "tst_test.h"

static void verify_execl(void)
{
	pid_t pid;
	char path[512];

	if (tst_get_path("execl01_child", path, sizeof(path)))
		tst_brk(TCONF, "Couldn't find execl01_child in $PATH");

	pid = SAFE_FORK();
	if (pid == 0) {
		TEST(execl(path, "execl01_child", "canary", NULL));
		tst_brk(TFAIL | TTERRNO,
			"Failed to execute execl01_child");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.test_all = verify_execl,
	.ulimit = (const struct tst_ulimit_val[]) {
		{RLIMIT_STACK, RLIM_INFINITY},
		{}
	},
};
