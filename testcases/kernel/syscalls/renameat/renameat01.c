/*
 * Copyright (c) International Business Machines  Corp., 2006
 *  Author: Yi Yang <yyangcdl@cn.ibm.com>
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 * Description:
 *   Verify that,
 *   1) renameat(2) returns -1 and sets errno to EBADF if olddirfd
 *      or newdirfd is not a valid file descriptor.
 *   2) renameat(2) returns -1 and sets errno to ENOTDIR if oldpath
 *      is relative and olddirfd is a file descriptor referring to
 *      a file other than a directory, or similar for newpath and
 *      newdirfd.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <error.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "lapi/renameat.h"

#define TESTDIR "testdir"
#define NEW_TESTDIR "new_testdir"
#define TESTFILE "testfile"
#define NEW_TESTFILE "new_testfile"
#define TESTFILE2 "testfile2"
#define NEW_TESTFILE2 "new_testfile2"
#define TESTFILE3 "testdir/testfile"
#define TESTFILE4 "testfile4"

#define DIRMODE (S_IRWXU | S_IRWXG | S_IRWXO)
#define FILEMODE (S_IRWXU | S_IRWXG | S_IRWXO)

static int curfd = AT_FDCWD;
static int olddirfd;
static int newdirfd;
static int badfd = 100;
static int filefd;
static char absoldpath[256];
static char absnewpath[256];

static struct test_case_t {
	int *oldfdptr;
	const char *oldpath;
	int *newfdptr;
	const char *newpath;
	int exp_errno;
} test_cases[] = {
	{ &curfd, TESTFILE, &curfd, NEW_TESTFILE, 0 },
	{ &olddirfd, TESTFILE, &newdirfd, NEW_TESTFILE, 0 },
	{ &olddirfd, absoldpath, &newdirfd, absnewpath, 0 },
	{ &badfd, TESTFILE, &badfd, NEW_TESTFILE, EBADF },
	{ &filefd, TESTFILE, &filefd, NEW_TESTFILE, ENOTDIR },
};

static void setup(void);
static void cleanup(void);
static void renameat_verify(const struct test_case_t *);

char *TCID = "renameat01";
int TST_TOTAL = ARRAY_SIZE(test_cases);
static int exp_enos[] = { EBADF, ENOTDIR, 0 };

int main(int ac, char **av)
{
	int i, lc;
	const char *msg;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			renameat_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	char *tmpdir;

	if ((tst_kvercmp(2, 6, 16)) < 0) {
		tst_brkm(TCONF, NULL,
			"This test can only run on kernels that are "
			"2.6.16 and higher");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;

	SAFE_TOUCH(cleanup, TESTFILE, FILEMODE, NULL);

	SAFE_TOUCH(cleanup, TESTFILE2, FILEMODE, NULL);
	tmpdir = tst_get_tmpdir();
	sprintf(absoldpath, "%s/%s", tmpdir, TESTFILE2);
	sprintf(absnewpath, "%s/%s", tmpdir, NEW_TESTFILE2);
	free(tmpdir);

	SAFE_MKDIR(cleanup, TESTDIR, DIRMODE);
	SAFE_TOUCH(cleanup, TESTFILE3, FILEMODE, NULL);
	SAFE_MKDIR(cleanup, NEW_TESTDIR, DIRMODE);

	olddirfd = SAFE_OPEN(cleanup, TESTDIR, O_DIRECTORY);
	newdirfd = SAFE_OPEN(cleanup, NEW_TESTDIR, O_DIRECTORY);

	filefd = SAFE_OPEN(cleanup, TESTFILE4,
				O_RDWR | O_CREAT, FILEMODE);
}

static void renameat_verify(const struct test_case_t *tc)
{
	TEST(renameat(*(tc->oldfdptr), tc->oldpath,
			*(tc->newfdptr), tc->newpath));

	if (tc->exp_errno && TEST_RETURN != -1) {
		tst_resm(TFAIL, "renameat() succeeded unexpectedly");
		return;
	}

	if (tc->exp_errno == 0 && TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "renameat() failed unexpectedly");
		return;
	}

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO,
		"renameat() returned the expected value");
	} else {
		tst_resm(TFAIL | TTERRNO,
			"renameat() got unexpected return value; expected: "
			"%d - %s", tc->exp_errno,
			strerror(tc->exp_errno));
	}

	if (TEST_ERRNO == 0 && renameat(*(tc->newfdptr), tc->newpath,
		*(tc->oldfdptr), tc->oldpath) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "renameat(%d, %s, "
			"%d, %s) failed.", *(tc->newfdptr), tc->newpath,
			*(tc->oldfdptr), tc->oldpath);
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (olddirfd && close(olddirfd) < 0)
		tst_resm(TWARN | TERRNO, "close olddirfd failed");

	if (newdirfd && close(newdirfd) < 0)
		tst_resm(TWARN | TERRNO, "close newdirfd failed");

	if (filefd && close(filefd) < 0)
		tst_resm(TWARN | TERRNO, "close filefd failed");

	tst_rmdir();
}
