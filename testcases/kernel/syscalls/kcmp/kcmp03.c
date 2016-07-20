/*
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
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
#include <sched.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "kcmp.h"
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
	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "kcmp() failed unexpectedly");
		return 0;
	}

	if (TEST_RETURN == 0)
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
	.tid = "kcmp03",
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.test = verify_kcmp,
	.min_kver = "3.5.0"
};
