/*
 *   Copyright (c) International Business Machines  Corp., 2006
 *   AUTHOR: Yi Yang <yyangcdl@cn.ibm.com>
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 * DESCRIPTION
 *	This test case will verify basic function of fchownat
 *	added by kernel 2.6.16 or up.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "safe_macros.h"
#include "fchownat.h"
#include "lapi/fcntl.h"

#define TESTFILE	"testfile"

static void setup(void);
static void cleanup(void);

static int dir_fd;
static int fd;
static int no_fd = -1;
static int cu_fd = AT_FDCWD;

static struct test_case_t {
	int exp_ret;
	int exp_errno;
	int flag;
	int *fds;
	char *filenames;
} test_cases[] = {
	{0, 0, 0, &dir_fd, TESTFILE},
	{-1, ENOTDIR, 0, &fd, TESTFILE},
	{-1, EBADF, 0, &no_fd, TESTFILE},
	{-1, EINVAL, 9999, &dir_fd, TESTFILE},
	{0, 0, 0, &cu_fd, TESTFILE},
};

char *TCID = "fchownat01";
int TST_TOTAL = ARRAY_SIZE(test_cases);
static void fchownat_verify(const struct test_case_t *);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			fchownat_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	dir_fd = SAFE_OPEN(cleanup, "./", O_DIRECTORY);

	SAFE_TOUCH(cleanup, TESTFILE, 0600, NULL);

	fd = SAFE_OPEN(cleanup, "testfile2", O_CREAT | O_RDWR, 0600);
}

static void fchownat_verify(const struct test_case_t *test)
{
	TEST(fchownat(*(test->fds), test->filenames, geteuid(),
		      getegid(), test->flag));

	if (TEST_RETURN != test->exp_ret) {
		tst_resm(TFAIL | TTERRNO,
			 "fchownat() returned %ld, expected %d, errno=%d",
			 TEST_RETURN, test->exp_ret, test->exp_errno);
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO,
			 "fchownat() returned the expected errno %d: %s",
			 test->exp_ret, strerror(test->exp_errno));
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "fchownat() failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);

	if (dir_fd > 0)
		close(dir_fd);

	tst_rmdir();
}
