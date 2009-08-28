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
 * Test Name: chmod01
 *
 * Test Description:
 *  Verify that, chmod(2) succeeds when used to change the mode permissions
 *  of a file.
 *
 * Expected Result:
 *  chmod(2) should return 0 and the mode permissions set on file should match
 *  the specified mode.
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
 *  chmod01 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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
 *  None.
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

#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TESTFILE	"testfile"

char *TCID = "chmod01";		/* Test program identifier.    */
int TST_TOTAL = 8;		/* Total number of test conditions */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int Modes[] = { 0, 07, 070, 0700, 0777, 02777, 04777, 06777 };

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int ind;		/* counter variable for chmod(2) tests */
	int mode;		/* file mode permission */

	TST_TOTAL = sizeof(Modes) / sizeof(int);

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			mode = Modes[ind];

			/*
			 * Call chmod(2) with different mode permission
			 * bits to set it for "testfile".
			 */
			TEST(chmod(TESTFILE, mode));

			/* check return code of chmod(2) */
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL|TTERRNO,
					 "chmod(%s, %#o) failed", TESTFILE, mode);
				continue;
			}
			/*
			 * Perform functional verification if test
			 * executed without (-f) option.
			 */
			if (STD_FUNCTIONAL_TEST) {
				/*
				 * Get the testfile information using
				 * stat(2).
				 */
				if (stat(TESTFILE, &stat_buf) < 0) {
					tst_brkm(TFAIL, cleanup, "stat(2) of "
						 "%s failed, errno:%d",
						 TESTFILE, TEST_ERRNO);
				}
				stat_buf.st_mode &= ~S_IFREG;

				/*
				 * Check for expected mode permissions
				 * on testfile.
				 */
				if (stat_buf.st_mode == mode) {
					tst_resm(TPASS, "Functionality of "
						 "chmod(%s, %#o) successful",
						 TESTFILE, mode);
				} else {
					tst_resm(TFAIL, "%s: Incorrect "
						 "modes 0%03o, Expected 0%03o",
						 TESTFILE, stat_buf.st_mode,
						 mode);
				}
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
 /*NOTREACHED*/}		/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and close it
 */
void setup()
{
	int fd;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Creat a test file under temporary directory and close it */
	fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) failed",
			 TESTFILE, FILE_MODE);
	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "close(%s) failed",
			 TESTFILE);

}				/* End setup() */

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
