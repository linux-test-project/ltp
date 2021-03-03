// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * This case is designed to test the basic functionality about the
 * O_CLOEXEC flag of pipe2.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "lapi/fcntl.h"
#include "tst_test.h"

#define TESTBIN "pipe2_02_child"
static int fds[2];

static void cleanup(void)
{
	if (fds[0] > 0)
		SAFE_CLOSE(fds[0]);
	if (fds[1] > 0)
		SAFE_CLOSE(fds[1]);
}

static void verify_pipe2(void)
{
	int pid, status;
	char buf[20];

	SAFE_PIPE2(fds, O_CLOEXEC);
	sprintf(buf, "%d", fds[1]);
	pid = SAFE_FORK();
	if (pid == 0)
		SAFE_EXECLP(TESTBIN, TESTBIN, buf, NULL);

	SAFE_WAIT(&status);
	if (WIFEXITED(status)) {
		switch (WEXITSTATUS(status)) {
		case 0:
			tst_res(TPASS, "test O_CLOEXEC for pipe2 success");
		break;
		case 1:
			tst_res(TFAIL, "test O_CLOEXEC for pipe2 failed");
		break;
		default:
			tst_brk(TBROK, "execlp() failed");
		}
	} else {
		tst_brk(TBROK, "%s exits with unexpected error", TESTBIN);
	}
	cleanup();
}

static struct tst_test test = {
	.resource_files = (const char *const []) {
		TESTBIN,
		NULL
	},
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.test_all = verify_pipe2,
};
