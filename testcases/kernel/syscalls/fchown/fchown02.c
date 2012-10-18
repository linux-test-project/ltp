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
 * Test Name: fchown02
 *
 * Test Description:
 *  Verify that, when fchown(2) invoked by super-user to change the owner and
 *  group of a file specified by file descriptor to any numeric
 *  owner(uid)/group(gid) values,
 *	- clears setuid and setgid bits set on an executable file.
 *	- preserves setgid bit set on a non-group-executable file.
 *
 * Expected Result:
 *  fchown(2) should return 0 and the ownership set on the file should match
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
 *  fchown02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

#define FILE_MODE	S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define NEW_PERMS1	S_IFREG | S_IRWXU | S_IRWXG | S_ISUID | S_ISGID
#define NEW_PERMS2	S_IFREG | S_IRWXU | S_ISGID
#define EXP_PERMS	S_IFREG | S_IRWXU | S_IRWXG
#define TESTFILE1	"testfile1"
#define TESTFILE2	"testfile2"

char *TCID = "fchown02";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test conditions */
int Fd1;			/* File descriptor for testfile1 */
int Fd2;			/* File descriptor for testfile2 */

int no_setup();
int setup1();			/* Test specific setup functions */
int setup2();

struct test_case_t {		/* test case struct. to for different tests */
	int fd;
	char *pathname;
	char *desc;
	uid_t user_id;
	gid_t group_id;
	int test_flag;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	1, TESTFILE1, "Setuid/Setgid bits cleared", 700, 701, 1, setup1}, {
	2, TESTFILE2, "Setgid bit not cleared", 700, 701, 2, setup2}, {
	0, NULL, NULL, 0, 0, 0, no_setup}
};

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int ind;		/* counter variable for chmod(2) tests */
	uid_t user_id;		/* user id of the user set for testfile */
	gid_t group_id;		/* group id of the user set for testfile */
	int fildes;		/* File descriptor for testfile */
	int test_flag;		/* test condition specific flag variable */
	char *file_name;	/* ptr. for test file name */
	char *test_desc;	/* test specific message */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			fildes = Test_cases[ind].fd;
			file_name = Test_cases[ind].pathname;
			test_desc = Test_cases[ind].desc;
			user_id = Test_cases[ind].user_id;
			group_id = Test_cases[ind].group_id;
			test_flag = Test_cases[ind].test_flag;

			if (fildes == 1) {
				fildes = Fd1;
			} else {
				fildes = Fd2;
			}

			/*
			 * Call fchwon(2) with different user id and
			 * group id (numeric values) to set it on
			 * testfile.
			 */
			TEST(fchown(fildes, user_id, group_id));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "fchown() Fails on %s, errno=%d",
					 file_name, TEST_ERRNO);
				continue;
			}
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Get the testfile information using
				 * fstat(2).
				 */
				if (fstat(fildes, &stat_buf) < 0) {
					tst_brkm(TFAIL, cleanup, "fstat(2) of "
						 "%s failed, errno=%d",
						 file_name, TEST_ERRNO);
				}

				/*
				 * Check for expected Ownership ids
				 * set on testfile.
				 */
				if ((stat_buf.st_uid != user_id) ||
				    (stat_buf.st_gid != group_id)) {
					tst_resm(TFAIL, "%s: Incorrect"
						 " ownership set, Expected %d "
						 "%d", file_name,
						 user_id, group_id);
				}

				/*
				 * Verify that S_ISUID/S_ISGID bits
				 * set on the testfile(s) in setup()s
				 * are cleared by chown().
				 */
				if ((test_flag == 1) && ((stat_buf.st_mode &
							  (S_ISUID | S_ISGID))))
				{
					tst_resm(TFAIL,
						 "%s: Incorrect mode "
						 "permissions %#o, Expected "
						 "%#o", file_name, NEW_PERMS1,
						 EXP_PERMS);
				} else if ((test_flag == 2)
					   && (!(stat_buf.st_mode & S_ISGID))) {
					tst_resm(TFAIL,
						 "%s: Incorrect mode "
						 "permissions %#o, Expected "
						 "%#o", file_name,
						 stat_buf.st_mode, NEW_PERMS2);
				} else {
					tst_resm(TPASS,
						 "fchown() on %s succeeds : %s",
						 file_name, test_desc);
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		}
	}

	cleanup();

	  return (0);
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Create a temporary directory and change directory to it.
 *	     Call test specific setup functions.
 */
void setup()
{
	int ind;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be super/root for this test!");
		tst_exit();
	}

	TEST_PAUSE;

	tst_tmpdir();

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}

/*
 * setup1() - Setup function for fchown(2) to verify setuid/setgid bits
 *	      set on an executable file will not be cleared.
 *	      Creat a testfile and set setuid/gid bits on it.
 */
int setup1()
{
	/* Creat a testfile under temporary directory */
	if ((Fd1 = open(TESTFILE1, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) Failed, errno=%d : %s",
			 TESTFILE1, FILE_MODE, errno, strerror(errno));
	}

	/* Set setuid/setgid bits on the test file created */
	if (chmod(TESTFILE1, NEW_PERMS1) != 0) {
		tst_brkm(TBROK, cleanup, "chmod(%s) Failed, errno=%d : %s",
			 TESTFILE1, errno, strerror(errno));
	}
	return 0;
}

/*
 * setup2() - Setup function for fchown(2) to verify setgid bit set
 *	      set on non-group executable file will not be cleared.
 *	      Creat a testfile and set setgid bit on it.
 */
int setup2()
{
	/* Creat a testfile under temporary directory */
	if ((Fd2 = open(TESTFILE2, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) Failed, errno=%d : %s",
			 TESTFILE2, FILE_MODE, errno, strerror(errno));
	}

	/* Set setgid bit on the test file created */
	if (chmod(TESTFILE2, NEW_PERMS2) != 0) {
		tst_brkm(TBROK, cleanup, "chmod(%s) Failed, errno=%d : %s",
			 TESTFILE2, errno, strerror(errno));
	}
	return 0;
}

/*
 * no_setup() - Some test conditions for mknod(2) do not any setup.
 *		Hence, this function just returns 0.
 *		This function simply returns 0.
 */
int no_setup()
{
	return 0;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *	       Close the temporary files.
 *	       Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the temporary file(s) opened in the setups  */
	if (close(Fd1) == -1) {
		tst_resm(TBROK, "close(%s) Failed, errno=%d : %s",
			 TESTFILE1, errno, strerror(errno));
	}
	if (close(Fd2) == -1) {
		tst_resm(TBROK, "close(%s) Failed, errno=%d : %s",
			 TESTFILE2, errno, strerror(errno));
	}

	tst_rmdir();

}
