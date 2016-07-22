/******************************************************************************
 *
 * Copyright (c) International Business Machines  Corp., 2006
 *  Author: Yi Yang <yyangcdl@cn.ibm.com>
 * Copyright (c) Cyril Hrubis 2014 <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * This test case will verify basic function of readlinkat
 * added by kernel 2.6.16 or up.
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/readlinkat.h"

static void setup(void);
static void cleanup(void);

char *TCID = "readlinkat01";

static int dir_fd, fd;
static int fd_invalid = 100;
static int fd_atcwd = AT_FDCWD;

#define TEST_SYMLINK "readlink_symlink"
#define TEST_FILE "readlink_file"

static char abspath[1024];

static struct test_case {
	int *dir_fd;
	const char *path;
	const char *exp_buf;
	int exp_ret;
	int exp_errno;
} test_cases[] = {
	{&dir_fd, TEST_SYMLINK, TEST_FILE, sizeof(TEST_FILE)-1, 0},
	{&dir_fd, abspath, TEST_FILE, sizeof(TEST_FILE)-1, 0},
	{&fd, TEST_SYMLINK, NULL, -1, ENOTDIR},
	{&fd_invalid, TEST_SYMLINK, NULL, -1, EBADF},
	{&fd_atcwd, TEST_SYMLINK, TEST_FILE, sizeof(TEST_FILE)-1, 0},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);

static void verify_readlinkat(struct test_case *test)
{
	char buf[1024];

	memset(buf, 0, sizeof(buf));

	TEST(readlinkat(*test->dir_fd, test->path, buf, sizeof(buf)));

	if (TEST_RETURN != test->exp_ret) {
		tst_resm(TFAIL | TTERRNO,
		         "readlinkat() returned %ld, expected %d",
		         TEST_RETURN, test->exp_ret);
		return;
	}

	if (TEST_ERRNO != test->exp_errno) {
		tst_resm(TFAIL | TTERRNO,
		         "readlinkat() returned %ld, expected %d",
		         TEST_RETURN, test->exp_ret);
		return;
	}

	if (test->exp_ret > 0 && strcmp(test->exp_buf, buf)) {
		tst_resm(TFAIL, "Unexpected buffer have '%s', expected '%s'",
		         buf, test->exp_buf);
		return;
	}

	tst_resm(TPASS | TTERRNO, "readlinkat() returned %ld", TEST_RETURN);
}

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < TST_TOTAL; i++)
			verify_readlinkat(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_tmpdir();
	char *tmpdir = tst_get_tmpdir();

	snprintf(abspath, sizeof(abspath), "%s/" TEST_SYMLINK, tmpdir);
	free(tmpdir);

	fd = SAFE_OPEN(cleanup, TEST_FILE, O_CREAT, 0600);
	SAFE_SYMLINK(cleanup, TEST_FILE, TEST_SYMLINK);
	dir_fd = SAFE_OPEN(cleanup, ".", O_DIRECTORY);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "Failed to close fd");

	if (dir_fd > 0 && close(dir_fd))
		tst_resm(TWARN | TERRNO, "Failed to close dir_fd");

	tst_rmdir();
}
