/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 */
 /* Description:
 *   Verify that:
 *              1) sched_setattr succeed with correct parameters
 *              2) sched_setattr fails with unused pid
 *              3) sched_setattr fails with invalid address
 *              4) sched_setattr fails with invalid flag
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>

#include "test.h"
#include "lapi/sched.h"

char *TCID = "sched_setattr01";

#define SCHED_DEADLINE	6
#define RUNTIME_VAL 10000000
#define PERIOD_VAL 30000000
#define DEADLINE_VAL 30000000

static pid_t pid;
static pid_t unused_pid;

static struct sched_attr attr = {
	.size = sizeof(struct sched_attr),
	.sched_flags = 0,
	.sched_nice = 0,
	.sched_priority = 0,

	.sched_policy = SCHED_DEADLINE,
	.sched_runtime = RUNTIME_VAL,
	.sched_period = PERIOD_VAL,
	.sched_deadline = DEADLINE_VAL,
};

static struct test_case {
	pid_t *pid;
	struct sched_attr *a;
	unsigned int flags;
	int exp_return;
	int exp_errno;
} test_cases[] = {
	{&pid, &attr, 0, 0, 0},
	{&unused_pid, &attr, 0, -1, ESRCH},
	{&pid, NULL, 0, -1, EINVAL},
	{&pid, &attr, 1000, -1, EINVAL}
};

static void setup(void);
static void sched_setattr_verify(const struct test_case *test);

int TST_TOTAL = ARRAY_SIZE(test_cases);

void *do_test(void *data LTP_ATTRIBUTE_UNUSED)
{
	int i;

	for (i = 0; i < TST_TOTAL; i++)
		sched_setattr_verify(&test_cases[i]);

	return NULL;
}

static void sched_setattr_verify(const struct test_case *test)
{
	TEST(sched_setattr(*(test->pid), test->a, test->flags));

	if (TEST_RETURN != test->exp_return) {
		tst_resm(TFAIL | TTERRNO, "sched_setattr(%i,attr,%u) "
		         "returned: %ld expected: %d",
		         *(test->pid), test->flags,
		         TEST_RETURN, test->exp_return);
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO,
			"sched_setattr() works as expected");
		return;
	}

	tst_resm(TFAIL | TTERRNO, "sched_setattr(%i,attr,%u): "
		"expected: %d - %s",
		*(test->pid), test->flags,
		test->exp_errno, tst_strerrno(test->exp_errno));
}

int main(int argc, char **argv)
{
	pthread_t thread;
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		pthread_create(&thread, NULL, do_test, NULL);
		pthread_join(thread, NULL);
	}

	tst_exit();
}

void setup(void)
{
	unused_pid = tst_get_unused_pid(setup);

	tst_require_root();

	if ((tst_kvercmp(3, 14, 0)) < 0)
		tst_brkm(TCONF, NULL, "EDF needs kernel 3.14 or higher");

	TEST_PAUSE;
}
