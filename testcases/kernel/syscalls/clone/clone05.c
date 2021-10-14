// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2012 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Call clone() with CLONE_VFORK flag set. verify that
 * execution of parent is suspended until child finishes
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <sched.h>
#include "tst_test.h"
#include "clone_platform.h"

static volatile int child_exited;
static void *child_stack;

static int child_fn(void *unused LTP_ATTRIBUTE_UNUSED)
{
	int i;

	for (i = 0; i < 100; i++) {
		sched_yield();
		usleep(1000);
	}

	child_exited = 1;
	_exit(0);
}

static void verify_clone(void)
{
	child_exited = 0;

	TST_EXP_PID_SILENT(ltp_clone(CLONE_VM | CLONE_VFORK | SIGCHLD, child_fn, NULL,
					CHILD_STACK_SIZE, child_stack), "clone with vfork");

	if (!TST_PASS)
		return;

	TST_EXP_VAL(child_exited, 1);
}

static struct tst_test test = {
	.test_all = verify_clone,
	.bufs = (struct tst_buffers []) {
		{&child_stack, .size = CHILD_STACK_SIZE},
		{},
	},
};
