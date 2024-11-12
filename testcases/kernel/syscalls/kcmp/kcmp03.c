// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Copyright (c) Linux Test Project, 2016-2024
 */

/*\
 * [Description]
 *
 * Verify that, kcmp() returns 0 if the processes
 *
 * 1. share the same file system information
 * 2. share I/O context
 * 3. share the same list of System V semaphore undo operations
 * 4. share the same address space
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/kcmp.h"
#include "lapi/sched.h"

#define STACK_SIZE	(1024*1024)

static int pid1;
static int pid2;
static void *stack;

#define ARGS(x, y) .clone_type = x, .kcmp_type = y, .desc = #x ", " #y
static struct tcase {
	int clone_type;
	int kcmp_type;
	char *desc;
} tcases[] = {
	{ARGS(CLONE_VM, KCMP_VM)},
	{ARGS(CLONE_FS, KCMP_FS)},
	{ARGS(CLONE_IO, KCMP_IO)},
	{ARGS(CLONE_SYSVSEM, KCMP_SYSVSEM)}
};

static void setup(void)
{
	stack = SAFE_MALLOC(STACK_SIZE);
}

static void cleanup(void)
{
	free(stack);
}

static int do_child(void *arg)
{
	pid2 = getpid();
	TST_EXP_PASS(kcmp(pid1, pid2, *(int *)arg, 0, 0));
	return 0;
}

static void verify_kcmp(unsigned int n)
{
	int res;
	struct tcase *tc = &tcases[n];

	pid1 = getpid();
	tst_res(TINFO, "Testing %s", tc->desc);

	res = ltp_clone(tc->clone_type | SIGCHLD, do_child, &tc->kcmp_type,
			STACK_SIZE, stack);
	if (res == -1)
		tst_res(TFAIL | TERRNO, "clone() Failed");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.test = verify_kcmp,
};
