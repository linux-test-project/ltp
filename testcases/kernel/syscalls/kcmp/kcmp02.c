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

#include "test.h"
#include "safe_macros.h"
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

char *TCID = "kcmp02";

#include <sys/types.h>
#include <sys/wait.h>

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

int TST_TOTAL = ARRAY_SIZE(test_cases);

static void cleanup(void);
static void setup(void);
static void kcmp_verify(const struct test_case *test);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			kcmp_verify(&test_cases[i]);

	}

	cleanup();
	tst_exit();
}

static void kcmp_verify(const struct test_case *test)
{
	TEST(kcmp(*(test->pid1), *(test->pid2), test->type,
			  *(test->fd1), *(test->fd2)));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "kcmp() succeeded unexpectedly");
		return;
	}

	if (test->exp_errno == TEST_ERRNO) {
		tst_resm(TPASS | TTERRNO, "kcmp() returned the expected value");
		return;
	}

	tst_resm(TFAIL | TTERRNO,
		"kcmp() got unexpected return value: expected: %d - %s",
			test->exp_errno, tst_strerrno(test->exp_errno));
}

static void setup(void)
{
	if ((tst_kvercmp(3, 5, 0)) < 0) {
		tst_brkm(TCONF, NULL,
			"This test can only run on kernels that are 3.5. and higher");
	}

	tst_tmpdir();

	pid1 = getpid();
	pid_unused = tst_get_unused_pid(cleanup);
	fd1 = SAFE_OPEN(cleanup, TEST_FILE, O_CREAT | O_RDWR | O_TRUNC);
	fd2 = SAFE_OPEN(cleanup, TEST_FILE2, O_CREAT | O_RDWR | O_TRUNC);

}

static void cleanup(void)
{
	if (fd1 > 0 && close(fd1) < 0)
		tst_resm(TWARN | TERRNO, "close fd1 failed");

	if (fd2 > 0 && close(fd2) < 0)
		tst_resm(TWARN | TERRNO, "close fd2 failed");
	tst_rmdir();

}
