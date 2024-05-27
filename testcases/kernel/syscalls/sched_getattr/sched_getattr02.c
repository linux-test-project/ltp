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
 *              1) sched_getattr fails with unused pid
 *              2) sched_getattr fails with invalid address
 *              3) sched_getattr fails with invalid value
 *              4) sched_getattr fails with invalid flag
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
#include <errno.h>

#include "test.h"
#include "lapi/sched.h"

char *TCID = "sched_getattr02";

static pid_t pid;
static pid_t unused_pid;
struct sched_attr attr_copy;

static struct test_case {
	pid_t *pid;
	struct sched_attr *a;
	unsigned int size;
	unsigned int flags;
	int exp_errno;
} test_cases[] = {
	{&unused_pid, &attr_copy, sizeof(struct sched_attr), 0, ESRCH},
	{&pid, NULL, sizeof(struct sched_attr), 0, EINVAL},
	{&pid, &attr_copy, sizeof(struct sched_attr) - 1, 0, EINVAL},
	{&pid, &attr_copy, sizeof(struct sched_attr), 1000, EINVAL}
};

static void setup(void);
static void sched_getattr_verify(const struct test_case *test);

int TST_TOTAL = ARRAY_SIZE(test_cases);

static void sched_getattr_verify(const struct test_case *test)
{
	TEST(sched_getattr(*(test->pid), test->a, test->size,
			test->flags));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "sched_getattr() succeeded unexpectedly.");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO,
			"sched_getattr() failed expectedly");
		return;
	}

	tst_resm(TFAIL | TTERRNO, "sched_getattr() failed unexpectedly "
		": expected: %d - %s",
		test->exp_errno, tst_strerrno(test->exp_errno));
}

int main(int argc, char **argv)
{
	int lc, i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < TST_TOTAL; i++)
			sched_getattr_verify(&test_cases[i]);
	}

	tst_exit();
}

void setup(void)
{
	unused_pid = tst_get_unused_pid(setup);

	TEST_PAUSE;
}
