// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2012 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Basic clone() test.
 *
 * Use clone() to create a child process, and wait for the child process to exit,
 * verify that the child process pid is correct.
 */

#include <stdlib.h>
#include "tst_test.h"
#include "clone_platform.h"

static void *child_stack;

static int do_child(void *arg LTP_ATTRIBUTE_UNUSED)
{
	exit(0);
}

static void verify_clone(void)
{
	int status, child_pid;

	TST_EXP_PID_SILENT(ltp_clone(SIGCHLD, do_child, NULL,
		CHILD_STACK_SIZE, child_stack), "clone()");

	child_pid = SAFE_WAIT(&status);

	if (child_pid == TST_RET)
		tst_res(TPASS, "clone returned %ld", TST_RET);
	else
		tst_res(TFAIL, "clone returned %ld, wait returned %d",
			 TST_RET, child_pid);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		tst_res(TPASS, "Child exited with 0");
	else
		tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = verify_clone,
	.forks_child = 1,
	.bufs = (struct tst_buffers []) {
		{&child_stack, .size = CHILD_STACK_SIZE},
		{}
	},
};
