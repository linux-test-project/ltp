// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 * Copyright (c) Linux Test Project, 2015-2024
 */

/*\
 * [Description]
 *
 * Verify that, kcmp() returns -1 and sets errno to
 *
 * 1. ESRCH if pid does not exist
 * 2. EINVAL if type is invalid (KCMP_TYPES + 1)
 * 3. EINVAL if type is invalid (-1)
 * 4. EINVAL if type is invalid (INT_MIN)
 * 5. EINVAL if type is invalid (INT_MAX)
 * 6. EBADF if file descriptor is invalid
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/kcmp.h"

#define TEST_FILE "test_file"
#define TEST_FILE2 "test_file2"

static int fd1;
static int fd2;
static int fd_fake;
static int pid1;
static int pid_unused;
static int fd_fake = -1;

#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

static struct test_case {
	int *pid1;
	int *pid2;
	int type;
	int *fd1;
	int *fd2;
	int exp_errno;
} test_cases[] = {
	{&pid1, &pid_unused, KCMP_FILE, &fd1, &fd2, ESRCH},
	{&pid1, &pid1, KCMP_TYPES + 1, &fd1, &fd2, EINVAL},
	{&pid1, &pid1, -1, &fd1, &fd2, EINVAL},
	{&pid1, &pid1, INT_MIN, &fd1, &fd2, EINVAL},
	{&pid1, &pid1, INT_MAX, &fd1, &fd2, EINVAL},
	{&pid1, &pid1, KCMP_FILE, &fd1, &fd_fake, EBADF}
};

static void setup(void)
{
	pid1 = getpid();
	pid_unused = tst_get_unused_pid();

	fd1 = SAFE_OPEN(TEST_FILE, O_CREAT | O_RDWR | O_TRUNC, 0644);
	fd2 = SAFE_OPEN(TEST_FILE2, O_CREAT | O_RDWR | O_TRUNC, 0644);
}

static void cleanup(void)
{
	if (fd1 > 0)
		SAFE_CLOSE(fd1);

	if (fd2 > 0)
		SAFE_CLOSE(fd2);
}

static void verify_kcmp(unsigned int n)
{
	struct test_case *test = &test_cases[n];

	TEST(kcmp(*(test->pid1), *(test->pid2), test->type,
		  *(test->fd1), *(test->fd2)));

	if (TST_RET != -1) {
		tst_res(TFAIL, "kcmp() succeeded unexpectedly");
		return;
	}

	if (test->exp_errno == TST_ERR) {
		tst_res(TPASS | TTERRNO, "kcmp() returned the expected value");
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"kcmp() got unexpected return value: expected: %d - %s",
			test->exp_errno, tst_strerrno(test->exp_errno));
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_kcmp,
	.needs_tmpdir = 1
};
