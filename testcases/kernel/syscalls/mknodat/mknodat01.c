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
 * DESCRIPTION
 *  This test case will verify basic function of mknodat
 *  added by kernel 2.6.16 or up.
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "tso_safe_macros.h"
#include "lapi/fcntl.h"

#define PATHNAME "mknodattestdir"

static void setup(void);
static void cleanup(void);
static void clean(void);

static char testfilepath[256];
static char testfile[256];
static char testfile2[256];
static char testfile3[256];

static int dir_fd, fd;
static int fd_invalid = 100;
static int fd_atcwd = AT_FDCWD;

static struct test_case {
	int *dir_fd;
	const char *name;
	int exp_ret;
	int exp_errno;
} test_cases[] = {
	{&dir_fd, testfile, 0, 0},
	{&dir_fd, testfile3, 0, 0},
	{&fd, testfile2, -1, ENOTDIR},
	{&fd_invalid, testfile, -1, EBADF},
	{&fd_atcwd, testfile, 0, 0}
};

char *TCID = "mknodat01";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static dev_t dev;

static void verify_mknodat(struct test_case *test)
{
	TEST(mknodat(*test->dir_fd, test->name, S_IFREG, dev));

	if (TEST_RETURN != test->exp_ret) {
		tst_resm(TFAIL | TTERRNO,
		         "mknodat() returned %ld, expected %d",
			 TEST_RETURN, test->exp_ret);
		return;
	}

	if (TEST_ERRNO != test->exp_errno) {
		tst_resm(TFAIL | TTERRNO,
		         "mknodat() returned wrong errno, expected %d",
			 test->exp_errno);
		return;
	}

	tst_resm(TPASS | TTERRNO, "mknodat() returned %ld", TEST_RETURN);
}

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			verify_mknodat(test_cases + i);

		/* clean created nodes before next run */
		clean();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	char *tmpdir;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Initialize test dir and file names */
	tmpdir = tst_get_tmpdir();
	sprintf(testfilepath, PATHNAME"/mknodattestfile%d", getpid());
	sprintf(testfile, "mknodattestfile%d", getpid());
	sprintf(testfile2, "mknodattestfile2%d", getpid());
	sprintf(testfile3, "%s/mknodattestfile3%d", tmpdir, getpid());
	free(tmpdir);

	SAFE_MKDIR(cleanup, PATHNAME, 0700);

	dir_fd = SAFE_OPEN(cleanup, PATHNAME, O_DIRECTORY);
	fd = SAFE_OPEN(cleanup, testfile2, O_CREAT | O_RDWR, 0600);
}

static void clean(void)
{
	SAFE_UNLINK(cleanup, testfilepath);
	SAFE_UNLINK(cleanup, testfile3);
	SAFE_UNLINK(cleanup, testfile);
}

static void cleanup(void)
{
	if (dir_fd > 0 && close(dir_fd))
		tst_resm(TWARN | TERRNO, "Failed to close(dir_fd)");

	if (fd > 0 && close(fd))
		tst_resm(TWARN | TERRNO, "Failed to close(fd)");

	tst_rmdir();
}
