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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tst_test.h"

#define IPC_ENV_VAR "LTP_IPC_PATH"

static void verify_execve(void)
{
	pid_t pid;
	char *const args[] = {"execve01_child", "canary", NULL};
	char path[512];

	char ipc_env_var[1024];
	sprintf(ipc_env_var, IPC_ENV_VAR "=%s", getenv(IPC_ENV_VAR));

	char *const envp[] = { "LTP_TEST_ENV_VAR=test", ipc_env_var, NULL };

	if (tst_get_path("execve01_child", path, sizeof(path)))
		tst_brk(TCONF, "Couldn't find execve01_child in $PATH");

	pid = SAFE_FORK();
	if (pid == 0) {
		execve(path, args, envp);
		tst_brk(TFAIL | TERRNO, "Failed to execute execl01_child");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.test_all = verify_execve,
};
