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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: chown02
 *
 * Test Description:
 *  Verify that, when chown(2) invoked by super-user to change the owner and
 *  group of a file specified by path to any numeric owner(uid)/group(gid)
 *  values,
 *	- clears setuid and setgid bits set on an executable file.
 *	- preserves setgid bit set on a non-group-executable file.
 *
 * Expected Result:
 *  chown(2) should return 0 and the ownership set on the file should match
 *  the numeric values contained in owner and group respectively.
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
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  chown02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

#define FILE_MODE	(S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define NEW_PERMS1	(S_IFREG|S_IRWXU|S_IRWXG|S_ISUID|S_ISGID)
#define NEW_PERMS2	(S_IFREG|S_IRWXU|S_ISGID)
#define EXP_PERMS	(S_IFREG|S_IRWXU|S_IRWXG)
#define TESTFILE1	"testfile1"
#define TESTFILE2	"testfile2"

TCID_DEFINE(chown02);

int setup1();			/* Test specific setup functions */
int setup2();

struct test_case_t {
	char *pathname;
	uid_t user_id;
	gid_t group_id;
	int test_flag;
	int (*setupfunc) ();
} test_cases[] = {
	/* setuid/setgid bits cleared */
	{
	TESTFILE1, 700, 701, 1, setup1},
	    /* setgid bit not cleared */
	{
TESTFILE2, 700, 701, 2, setup2},};

int TST_TOTAL = ARRAY_SIZE(test_cases);

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;
	int i;
	uid_t user_id;		/* user id of the user set for testfile */
	gid_t group_id;		/* group id of the user set for testfile */
	int test_flag;		/* test condition specific flag variable */
	char *file_name;	/* ptr. for test file name */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			file_name = test_cases[i].pathname;
			user_id = test_cases[i].user_id;
			group_id = test_cases[i].group_id;
			test_flag = test_cases[i].test_flag;

			/*
			 * Call chown(2) with different user id and
			 * group id (numeric values) to set it on testfile.
			 */
			TEST(CHOWN(cleanup, file_name, user_id, group_id));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL | TTERRNO,
					 "chown(%s, ..) failed", file_name);
				continue;
			}

			/*
			 * Get the testfile information using stat(2).
			 */
			if (stat(file_name, &stat_buf) < 0) {
				tst_brkm(TFAIL, cleanup, "stat(2) of "
					 "%s failed, errno:%d",
					 file_name, TEST_ERRNO);
			}

			/*
			 * Check for expected Ownership ids
			 * set on testfile.
			 */
			if (stat_buf.st_uid != user_id ||
			    stat_buf.st_gid != group_id) {
				tst_brkm(TFAIL, cleanup, "%s: incorrect"
					 " ownership set, Expected %d "
					 "%d", file_name,
					 user_id, group_id);
			}

			/*
			 * Verify that S_ISUID/S_ISGID bits set on the
			 * testfile(s) in setup()s are cleared by
			 * chown().
			 */
			if (test_flag == 1 &&
			    (stat_buf.st_mode & (S_ISUID | S_ISGID)) != 0) {
				tst_resm(TFAIL,
					 "%s: incorrect mode "
					 "permissions %#o, Expected "
					 "%#o", file_name, NEW_PERMS1,
					 EXP_PERMS);
			} else if (test_flag == 2
				 && (stat_buf.st_mode & S_ISGID) == 0) {
				tst_resm(TFAIL,
					 "%s: Incorrect mode "
					 "permissions %#o, Expected "
					 "%#o", file_name,
					 stat_buf.st_mode, NEW_PERMS2);
			} else {
				tst_resm(TPASS,
					 "chown(%s, ..) succeeded",
					 file_name);
			}
		}
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and close it
 */
void setup(void)
{
	int i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	/* call iividual setup functions */
	for (i = 0; i < TST_TOTAL; i++)
		test_cases[i].setupfunc();
}

/*
 * int
 * setup1() - Setup function for chown(2) to verify setuid/setgid bits
 *	      set on an executable file will not be cleared.
 *  Creat a testfile and set setuid/setgid bits on the mode of file.$
 */
int setup1(void)
{
	int fd;			/* File descriptor for testfile1 */

	/* Creat a testfile and close it */
	if ((fd = open(TESTFILE1, O_RDWR | O_CREAT, FILE_MODE)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) failed",
			 TESTFILE1, FILE_MODE);
	SAFE_CLOSE(cleanup, fd);

	/* Set setuid/setgid bits on the test file created */
	SAFE_CHMOD(cleanup, TESTFILE1, NEW_PERMS1);
	return 0;
}

/*
 * int
 * setup2() - Setup function for chown(2) to verify setgid bit set
 *	      set on non-group executable file will not be cleared.
 *  Creat a testfile and set setgid bit on the mode of file.
 */
int setup2(void)
{
	int fd;			/* File descriptor for testfile2 */

	/* Creat a testfile and close it */
	if ((fd = open(TESTFILE2, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) failed",
			 TESTFILE2, FILE_MODE);
	}
	/* Set setgid bit on the test file created */
	if (fchmod(fd, NEW_PERMS2) != 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fchmod failed");
	SAFE_CLOSE(cleanup, fd);
	return 0;
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup(void)
{

	tst_rmdir();

}
