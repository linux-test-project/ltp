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
 */

/*
 * DESCRIPTION
 *	This test case will verify basic function of mkdirat
 *	added by kernel 2.6.16 or up.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "lapi/mkdirat.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

static char relpath[256];
static char abspath[1024];
static int dir_fd, fd;
static int fd_invalid = 100;
static int fd_atcwd = AT_FDCWD;

static struct test_case {
	int *dir_fd;
	const char *name;
	int exp_ret;
	int exp_errno;
} test_cases[] = {
	{&dir_fd, relpath, 0, 0},
	{&dir_fd, abspath, 0, 0},
	{&fd_atcwd, relpath, 0, 0},
	{&fd, relpath, -1, ENOTDIR},
	{&fd_invalid, relpath, -1, EBADF},
};

char *TCID = "mkdirat01";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static void verify_mkdirat(struct test_case *test)
{
	TEST(mkdirat(*test->dir_fd, test->name, 0600));

	if (TEST_RETURN != test->exp_ret) {
		tst_resm(TFAIL | TTERRNO,
		         "mkdirat() returned %ld, expected %d",
			 TEST_RETURN, test->exp_ret);
		return;
	}

	if (TEST_ERRNO != test->exp_errno) {
		tst_resm(TFAIL | TTERRNO,
		         "mkdirat() returned wrong errno, expected %d",
			 test->exp_errno);
		return;
	}

	tst_resm(TPASS | TTERRNO, "mkdirat() returned %ld", TEST_RETURN);
}

static void setup_iteration(int i)
{
	static char testdir[256];
	char *tmpdir = tst_get_tmpdir();

	/* Initialize test dir and file names */
	sprintf(testdir, "mkdirattestdir%d_%d", getpid(), i);
	sprintf(relpath, "mkdiratrelpath%d_%d", getpid(), i);
	sprintf(abspath, "%s/mkdiratrelpath%d_%d_2", tmpdir, getpid(), i);

	free(tmpdir);

	SAFE_MKDIR(cleanup, testdir, 0700);
	dir_fd = SAFE_OPEN(cleanup, testdir, O_DIRECTORY);
}

static void cleanup_iteration(void)
{
	SAFE_CLOSE(cleanup, dir_fd);
}

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		setup_iteration(lc);

		for (i = 0; i < TST_TOTAL; i++)
			verify_mkdirat(test_cases + i);

		cleanup_iteration();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;
	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, "mkdirattestfile", O_CREAT | O_RDWR, 0600);
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);

	tst_rmdir();
}
