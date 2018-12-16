/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
 *    DATE STARTED	: 06/01/02
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tst_test.h"

#define IPC_ENV_VAR "LTP_IPC_PATH"

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
		tst_brk(TFAIL | TERRNO,
			"Failed to execute execl01_child");
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.child_needs_reinit = 1,
	.test_all = verify_execle,
};
