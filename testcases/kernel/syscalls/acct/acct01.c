/*
 *
 *  Copyright (c) International Business Machines  Corp., 2002
 *
 *  This program is free software;  you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *  the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* 12/03/2002	Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * ALGORITHM
 *	issue calls to acct and test the returned values against
 *	expected results
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

#define TEST_FILE1	"/"
#define TEST_FILE2	"/dev/null"
#define TEST_FILE3	"/tmp/does/not/exist"
#define TEST_FILE4	"/etc/fstab/"
#define TEST_FILE5	"./tmpfile"

static void setup(void);
static void cleanup(void);
static void setup2(void);
static void cleanup2(void);
static void acct_verify(int);

static struct test_case_t {
	char *filename;
	char *exp_errval;
	int exp_errno;
	void (*setupfunc) ();
	void (*cleanfunc) ();
} test_cases[] = {
	{TEST_FILE1, "EISDIR",  EISDIR,  NULL,   NULL},
	{TEST_FILE2, "EACCES",  EACCES,  NULL,   NULL},
	{TEST_FILE3, "ENOENT",  ENOENT,  NULL,   NULL},
	{TEST_FILE4, "ENOTDIR", ENOTDIR, NULL,   NULL},
	{TEST_FILE5, "EPERM",   EPERM,   setup2, cleanup2},
};

char *TCID = "acct01";
int TST_TOTAL = ARRAY_SIZE(test_cases);
static struct passwd *ltpuser;
static int exp_enos[] = { EISDIR, EACCES, ENOENT, ENOTDIR, EPERM, 0 };

int main(int argc, char *argv[])
{
	int lc;
	int i;

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			acct_verify(i);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;

	tst_require_root(NULL);

	tst_tmpdir();

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	fd = SAFE_CREAT(cleanup, TEST_FILE5, 0777);
	SAFE_CLOSE(cleanup, fd);

	if (acct(TEST_FILE5) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "acct failed unexpectedly");

	/* turn off acct, so we are in a known state */
	if (acct(NULL) == -1) {
		if (errno == ENOSYS) {
			tst_brkm(TCONF, cleanup,
				 "BSD process accounting is not configured in "
				 "this kernel");
		} else {
			tst_brkm(TBROK | TERRNO, cleanup, "acct(NULL) failed");
		}
	}
}

static void acct_verify(int i)
{

	if (test_cases[i].setupfunc)
		test_cases[i].setupfunc();

	TEST(acct(test_cases[i].filename));

	if (test_cases[i].cleanfunc)
		test_cases[i].cleanfunc();

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "acct(%s) succeeded unexpectedly",
			 test_cases[i].filename);
		return;
	}
	if (TEST_ERRNO == test_cases[i].exp_errno) {
		tst_resm(TPASS | TTERRNO, "acct failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "acct failed unexpectedly; expected: %d - %s",
			 test_cases[i].exp_errno,
			 strerror(test_cases[i].exp_errno));
	}
}

static void setup2(void)
{
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void cleanup2(void)
{
	SAFE_SETEUID(cleanup, 0);
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (acct(NULL) == -1)
		tst_resm(TBROK | TERRNO, "acct(NULL) failed");

	tst_rmdir();
}
