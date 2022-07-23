// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) International Business Machines  Corp., 2003.
 * Copyright (c) 2012 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Test for a libc bug where exiting child function by returning from
 * it caused SIGSEGV.
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "clone_platform.h"

static void *child_stack;

static int do_child(void *arg LTP_ATTRIBUTE_UNUSED)
{
	return 0;
}

static void verify_clone(void)
{
	int status;
	pid_t pid = 0;

	TST_EXP_PID_SILENT(ltp_clone(SIGCHLD, do_child, NULL, CHILD_STACK_SIZE,
				child_stack));

	if (!TST_PASS)
		return;

	SAFE_WAITPID(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		tst_res(TPASS, "Using return in child will not cause abnormal exit");
		return;
	}

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
		tst_res(TFAIL, "Use of return in child caused SIGSEGV");
		return;
	}

	tst_res(TFAIL, "Using return 0 in child the child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = verify_clone,
	.bufs = (struct tst_buffers []) {
		{&child_stack, .size = CHILD_STACK_SIZE},
		{},
	},
};
