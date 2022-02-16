// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 */

 /* Description:
 *   Verify that:
 *		1) kcmp returns 0 with two process and two fd refering to the
 *			same open file
 *		2) kcmp doesn't return 0 with two process and two fd not
 *		   refering to the same open file
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/kcmp.h"

#define TEST_FILE "test_file"
#define TEST_FILE2 "test_file2"

static int fd1;
static int fd2;
static int fd3;
static int pid1;
static int pid2;

static struct test_case {
	int *pid1;
	int *pid2;
	int type;
	int *fd1;
	int *fd2;
	int exp_different;
} test_cases[] = {
	{&pid1, &pid1, KCMP_FILE, &fd1, &fd1, 0},
	{&pid2, &pid2, KCMP_FILE, &fd1, &fd2, 0},
	{&pid1, &pid2, KCMP_FILE, &fd1, &fd1, 0},
	{&pid1, &pid2, KCMP_FILE, &fd1, &fd2, 0},
	{&pid1, &pid2, KCMP_FILE, &fd1, &fd3, 1},
};

static void setup(void)
{
	fd1 = SAFE_OPEN(TEST_FILE, O_CREAT | O_RDWR | O_TRUNC, 0666);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);
}

static void do_child(const struct test_case *test)
{
	pid2 = getpid();

	fd3 = SAFE_OPEN(TEST_FILE2, O_CREAT | O_RDWR, 0666);

	fd2 = dup(fd1);
	if (fd2 == -1) {
		tst_res(TFAIL | TERRNO, "dup() failed unexpectedly");
		SAFE_CLOSE(fd3);
		return;
	}

	TEST(kcmp(*(test->pid1), *(test->pid2), test->type,
		  *(test->fd1), *(test->fd2)));

	SAFE_CLOSE(fd2);
	SAFE_CLOSE(fd3);

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "kcmp() failed unexpectedly");
		return;
	}

	if ((test->exp_different && TST_RET == 0)
		|| (test->exp_different == 0 && TST_RET)) {
		tst_res(TFAIL, "kcmp() returned %lu instead of %d",
			TST_RET, test->exp_different);
		return;
	}

	tst_res(TPASS, "kcmp() returned the expected value");
}

static void verify_kcmp(unsigned int n)
{
	struct test_case *tc = &test_cases[n];

	pid1 = getpid();

	pid2 = SAFE_FORK();
	if (!pid2)
		do_child(tc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.test = verify_kcmp,
	.min_kver = "3.5.0",
	.needs_tmpdir = 1,
};
