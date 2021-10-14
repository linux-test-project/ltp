// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2012 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Check for equality of getpid() from a child and return value of clone(2)
 */

#include <stdio.h>
#include <stdlib.h>
#include "tst_test.h"
#include "clone_platform.h"

static void *child_stack;
static int *child_pid;

static int child_fn(void *arg LTP_ATTRIBUTE_UNUSED)
{
	*child_pid = getpid();
	exit(0);
}

static void verify_clone(void)
{

	TST_EXP_PID_SILENT(ltp_clone(SIGCHLD, child_fn, NULL, CHILD_STACK_SIZE,
				child_stack));

	if (!TST_PASS)
		return;

	tst_reap_children();

	TST_EXP_VAL(TST_RET, *child_pid, "pid(%d)", *child_pid);
}

static void setup(void)
{
	child_pid = SAFE_MMAP(NULL, sizeof(*child_pid), PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (child_pid)
		SAFE_MUNMAP(child_pid, sizeof(*child_pid));
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_clone,
	.bufs = (struct tst_buffers []) {
		{&child_stack, .size = CHILD_STACK_SIZE},
		{},
	},
	.cleanup = cleanup,
};
