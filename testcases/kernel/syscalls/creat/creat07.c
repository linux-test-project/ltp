/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2012-2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Testcase to check creat(2) sets ETXTBSY correctly.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "tst_test.h"

#define TEST_APP "creat07_child"

static void verify_creat(void)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		char *av[] = {TEST_APP, NULL};
		(void)execve(TEST_APP, av, tst_ipc_envp);
		perror("execve failed");
		exit(1);
	}

	TST_CHECKPOINT_WAIT(0);

	TEST(creat(TEST_APP, O_WRONLY));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "creat() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == ETXTBSY)
		tst_res(TPASS, "creat() received EXTBSY");
	else
		tst_res(TFAIL | TTERRNO, "creat() failed unexpectedly");

	SAFE_KILL(pid, SIGKILL);
	SAFE_WAITPID(pid, NULL, 0);
}

static const char *const resource_files[] = {
	TEST_APP,
	NULL,
};

static struct tst_test test = {
	.tid = "creat07",
	.test_all = verify_creat,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.resource_files = resource_files,
};
