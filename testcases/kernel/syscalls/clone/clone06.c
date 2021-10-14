// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2012 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Test to verify inheritance of environment variables by child.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include "tst_test.h"
#include "clone_platform.h"

#define MAX_LINE_LENGTH 256
#define ENV_VAL "LTP test variable value"
#define ENV_ID "LTP_CLONE_TEST"

static void *child_stack;

static int child_environ(void *arg LTP_ATTRIBUTE_UNUSED)
{
	const char *env_val = getenv(ENV_ID);
	if (!env_val) {
		tst_res(TFAIL, "Variable " ENV_ID " not defined in child");
		exit(0);
	}

	if (strcmp(env_val, ENV_VAL)) {
		tst_res(TFAIL, "Variable value is different");
		exit(0);
	}

	tst_res(TPASS, "The environment variables of the child and the parent are the same ");

	exit(0);
}

static void verify_clone(void)
{
	TST_EXP_PID_SILENT(ltp_clone(SIGCHLD, child_environ, NULL, CHILD_STACK_SIZE,
				child_stack));

	if (!TST_PASS)
		return;

	tst_reap_children();
}

static void setup(void)
{
	SAFE_SETENV(ENV_ID, ENV_VAL, 0);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_clone,
	.bufs = (struct tst_buffers []) {
		{&child_stack, .size = CHILD_STACK_SIZE},
		{},
	},
};
