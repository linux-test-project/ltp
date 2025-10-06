// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018-2025 Linux Test Project
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Authors: William Roske, Dave Fenner
 */

#include <string.h>
#include <stdio.h>
#include "tst_test.h"

/*\
 * Test :man2:`execve` passes correctly argv[1] and environment variable to the
 * executed binary.
 */

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
		tst_brk(TFAIL | TERRNO, "Failed to execute execve01_child");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.test_all = verify_execve,
};
