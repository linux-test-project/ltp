// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "tst_test.h"

static void verify_execlp(void)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		TEST(execlp("execlp01_child", "execlp01_child", "canary", NULL));
		tst_brk(TFAIL | TTERRNO,
			"Failed to execute execlp01_child");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.test_all = verify_execlp,
};
