/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
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
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

 /* Description:
 *   Verify that:
 *		1) kcmp fails with bad pid
 *		2) kcmp fails with invalid flag
 *		3) kcmp fails with invalid flag
 *		4) kcmp fails with invalid flag
 *		5) kcmp fails with invalid flag
 *		6) kcmp fails with invalid fd
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "kcmp.h"

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

	fd1 = SAFE_OPEN(TEST_FILE, O_CREAT | O_RDWR | O_TRUNC);
	fd2 = SAFE_OPEN(TEST_FILE2, O_CREAT | O_RDWR | O_TRUNC);
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
	.min_kver = "3.5.0",
	.needs_tmpdir = 1
};
