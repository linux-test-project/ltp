/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: fchmod06
 *
 * Test Description:
 *   Verify that,
 *   1) fchmod(2) returns -1 and sets errno to EPERM if the effective user id
 *	of process does not match the owner of the file and the process is
 *	not super user.
 *   2) fchmod(2) returns -1 and sets errno to EBADF if the file descriptor
 *	of the specified file is not valid.
 *
 * Expected Result:
 *  fchmod() should fail with return value -1 and set expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  fchmod06 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be executed by 'non-super-user' only.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define MODE_RWX	(S_IRWXU|S_IRWXG|S_IRWXO)
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define TEST_FILE1	"tfile_1"
#define TEST_FILE2	"tfile_2"

void setup1();			/* setup function to test chmod for EPERM */
void setup2();			/* setup function to test chmod for EBADF */

int fd1;			/* File descriptor for testfile1 */
int fd2;			/* File descriptor for testfile2 */

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	int fd;
	int mode;
	int exp_errno;
	void (*setupfunc)();
} test_cases[] = {
	{ 1, FILE_MODE, EPERM, setup1 },
	{ 2, FILE_MODE, EBADF, setup2 },
};

char *TCID = "fchmod06";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
int exp_enos[] = { EPERM, EBADF, 0 };

char nobody_uid[] = "nobody";
struct passwd *ltpuser;
char *test_home;		/* variable to hold TESTHOME env. */

void setup();			/* Main setup function for the tests */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int fd;			/* test file descriptor */
	int i;		/* counter to test different test conditions */
	int mode;		/* creation mode for the node created */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			fd = test_cases[i].fd;
			mode = test_cases[i].mode;

			if (fd == 1)
				fd = fd1;
			else
				fd = fd2;

			TEST(fchmod(fd, mode));

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == test_cases[i].exp_errno)
					tst_resm(TPASS|TTERRNO,
					    "fchmod failed as expected");
				else
					tst_resm(TFAIL|TTERRNO,
					    "fchmod failed unexpectedly");
			} else
				tst_resm(TFAIL,
				    "fchmod succeeded unexpectedly");
		}

	}

	cleanup();
	tst_exit();
}

void setup()
{
	int i;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);
	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getpwnam failed");
	if (seteuid(ltpuser->pw_uid) == -1)
		tst_resm(TBROK|TERRNO, "seteuid failed");

	test_home = get_current_dir_name();

	TEST_PAUSE;

	tst_tmpdir();

	for (i = 0; i < TST_TOTAL; i++)
		test_cases[i].setupfunc();
}

void setup1()
{
	uid_t old_uid;

	if ((fd1 = open(TEST_FILE1, O_RDWR|O_CREAT, 0666)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open(%s, ..) failed",
		    TEST_FILE1);

	old_uid = geteuid();
	if (seteuid(0) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "seteuid(0) failed");

	if (fchown(fd1, 0, 0) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "fchown of %s failed",
		    TEST_FILE1);

	if (seteuid(old_uid) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "seteuid(%d) failed", old_uid);

}

void setup2()
{
	if ((fd2 = open(TEST_FILE2, O_RDWR|O_CREAT, 0666)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open(%s, ..) failed",
		    TEST_FILE2);
	if (close(fd2) == -1)
		tst_brkm(TBROK, cleanup, "closing %s failed", TEST_FILE2);
}

void cleanup()
{
	TEST_CLEANUP;

	if (close(fd1) == -1)
		tst_resm(TWARN|TERRNO, "closing %s failed", TEST_FILE1);

	tst_rmdir();
}