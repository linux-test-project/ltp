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
 * Test Name: fchown05
 *
 * Test Description:
 *  Verify that, fchown(2) succeeds to change the owner and group of a file
 *  specified by file descriptor to any numeric owner(uid)/group(gid) values
 *  when invoked by super-user.
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
 *  fchown05 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#define TESTFILE	"testfile"

int fildes;			/* File descriptor for the test file */
char *TCID = "fchown05";	/* Test program identifier.    */
int TST_TOTAL = 5;		/* Total number of test conditions */

struct test_case_t {		/* Struct. for for test case looping */
	char *desc;
	uid_t user_id;
	gid_t group_id;
} Test_cases[] = {
	{
	"Change Owner/Group ids", 700, 701}, {
	"Change Owner id only", 702, -1}, {
	"Change Owner id only", 703, 701}, {
	"Change Group id only", -1, 704}, {
	"Change Group id only", 703, 705}, {
	NULL, 0, 0}
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
			test_desc = Test_cases[ind].desc;
			user_id = Test_cases[ind].user_id;
			group_id = Test_cases[ind].group_id;

			/*
			 * Call fchwon(2) with different user id and
			 * group id (numeric values) to set it on
			 * testfile.
			 */
			TEST(fchown(fildes, user_id, group_id));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "fchown() Fails to %s, errno=%d",
					 test_desc, TEST_ERRNO);
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
						 "%s failed, errno:%d",
						 TESTFILE, TEST_ERRNO);
				}
				if (user_id == -1) {
					user_id = Test_cases[ind - 1].user_id;
				}
				if (group_id == -1) {
					group_id = Test_cases[ind - 1].group_id;
				}

				/*
				 * Check for expected Ownership ids
				 * set on testfile.
				 */
				if ((stat_buf.st_uid != user_id) ||
				    (stat_buf.st_gid != group_id)) {
					tst_resm(TFAIL, "%s: Incorrect owner"
						 "ship set, Expected %d %d",
						 TESTFILE, user_id, group_id);
				} else {
					tst_resm(TPASS,
						 "fchown() succeeds to %s of %s",
						 test_desc, TESTFILE);
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
 *	     Create a test file under temporary directory.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be super/root for this test!");
		tst_exit();
	}

	TEST_PAUSE;

	tst_tmpdir();

	if ((fildes = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) Failed, errno=%d : %s",
			 TESTFILE, FILE_MODE, errno, strerror(errno));
	}

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *	       Close the testfile opened in the setup.
 *	       Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the testfile */
	if (close(fildes) == -1) {
		tst_resm(TBROK, "close(%s) Failed, errno=%d : %s",
			 TESTFILE, errno, strerror(errno));
	}

	tst_rmdir();

}