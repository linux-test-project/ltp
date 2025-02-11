// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Test that kernel adds dummy argv[0] if empty argument list was passed to
 * execve(). This fixes at least one CVE where userspace programs start to
 * process argument list blindly from argv[1] such as polkit pkexec
 * CVE-2021-4034.
 *
 * See also https://lwn.net/Articles/883547/
 */

#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"

static void verify_execve(void)
{
	pid_t pid;
	char path[512];
	char ipc_env_var[1024];

	sprintf(ipc_env_var, IPC_ENV_VAR "=%s", getenv(IPC_ENV_VAR));

	char *const envp[] = {ipc_env_var, NULL};
	char *const argv[] = {NULL};

	if (tst_get_path("execve06_child", path, sizeof(path)))
		tst_brk(TCONF, "Couldn't find execve06_child in $PATH");

	pid = SAFE_FORK();
	if (pid == 0) {
		execve(path, argv, envp);
		tst_brk(TFAIL | TERRNO, "Failed to execute execve06_child");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.test_all = verify_execve,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "dcd46d897adb"},
		{"CVE", "2021-4034"},
		{}
	}
};
