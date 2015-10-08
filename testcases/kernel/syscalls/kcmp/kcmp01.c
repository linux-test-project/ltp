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
 *		1) kcmp returns 0 with two process and two fd refering to the
 *			same open file
 *		2) kcmp doesn't return 0 with two process and two fd not
 *		   refering to the same open file
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
static int fd3;
static int pid1;
static int pid2;

char *TCID = "kcmp01";

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

int TST_TOTAL = ARRAY_SIZE(test_cases);

static void cleanup(void);
static void setup(void);
static void do_child(const struct test_case *test);
static void cleanup_child(void);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			pid2 = tst_fork();

			if (pid2 == -1)
				tst_brkm(TBROK, cleanup, "fork failed");

			if (!pid2)
				do_child(&test_cases[i]);
			else
				tst_record_childstatus(cleanup, pid2);
			tst_count++;
		}
	}

	cleanup();
	tst_exit();
}

static void do_child(const struct test_case *test)
{
	pid2 = getpid();
	fd2 = dup(fd1);
	fd3 = SAFE_OPEN(cleanup_child, TEST_FILE2, O_CREAT | O_RDWR, 0666);

	TEST(kcmp(*(test->pid1), *(test->pid2), test->type,
			  *(test->fd1), *(test->fd2)));

	if (TEST_RETURN == -1)
		tst_resm(TFAIL | TTERRNO, "kcmp() failed unexpectedly");

	if ((test->exp_different && TEST_RETURN == 0)
		|| (test->exp_different == 0 && TEST_RETURN))
		tst_resm(TFAIL, "kcmp() returned %lu instead of %d",
				TEST_RETURN, test->exp_different);

	if ((test->exp_different == 0 && TEST_RETURN == 0)
		|| (test->exp_different && TEST_RETURN))
		tst_resm(TPASS, "kcmp() returned the expected value");

	tst_exit();
}

static void cleanup_child(void)
{
	if (fd2 > 0 && close(fd2) < 0)
		tst_resm(TWARN | TERRNO, "close fd2 failed");
	fd2 = 0;
	if (fd3 > 0 && close(fd3) < 0)
		tst_resm(TWARN | TERRNO, "close fd3 failed");
	fd3 = 0;
}

static void setup(void)
{
	if ((tst_kvercmp(3, 5, 0)) < 0) {
		tst_brkm(TCONF, NULL,
			"This test can only run on kernels that are 3.5. and higher");
	}

	tst_tmpdir();

	pid1 = getpid();
	fd1 = SAFE_OPEN(cleanup, TEST_FILE, O_CREAT | O_RDWR | O_TRUNC);
}

static void cleanup(void)
{
	if (fd1 > 0 && close(fd1) < 0)
		tst_resm(TWARN | TERRNO, "close fd1 failed");
	tst_rmdir();
}
