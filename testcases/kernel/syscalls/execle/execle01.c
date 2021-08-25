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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tst_test.h"

static void verify_execle(void)
{
	pid_t pid;
	char path[512];
	char ipc_env_var[1024];

	sprintf(ipc_env_var, IPC_ENV_VAR "=%s", getenv(IPC_ENV_VAR));
	char *const envp[] = { "LTP_TEST_ENV_VAR=test", ipc_env_var, NULL };

	if (tst_get_path("execle01_child", path, sizeof(path)))
		tst_brk(TCONF, "Couldn't find execle01_child in $PATH");

	pid = SAFE_FORK();
	if (pid == 0) {
		TEST(execle(path, "execle01_child", "canary", NULL, envp));
		tst_brk(TFAIL | TTERRNO,
			"Failed to execute execl01_child");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.test_all = verify_execle,
};
