// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

 /*
 * Testname: kcmp03.c
 *
 * Description:
 * 1) kcmp() returns 0 if the processes share the same file system information.
 * 2) kcmp() returns 0 if the processes share I/O context.
 * 3) kcmp() returns 0 if the processes share the same list of System V
 *    semaphore undo operations.
 * 4) kcmp() returns 0 if the processes share the same address space.
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

static struct tcase {
	int clone_type;
	int kcmp_type;
} tcases[] = {
	{CLONE_VM, KCMP_VM},
	{CLONE_FS, KCMP_FS},
	{CLONE_IO, KCMP_IO},
	{CLONE_SYSVSEM, KCMP_SYSVSEM}
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

	TEST(kcmp(pid1, pid2, *(int *)arg, 0, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "kcmp() failed unexpectedly");
		return 0;
	}

	if (TST_RET == 0)
		tst_res(TPASS, "kcmp() returned the expected value");
	else
		tst_res(TFAIL, "kcmp() returned the unexpected value");

	return 0;
}

static void verify_kcmp(unsigned int n)
{
	int res;

	struct tcase *tc = &tcases[n];

	pid1 = getpid();

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
	.min_kver = "3.5.0"
};
