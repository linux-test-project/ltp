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
 * Test Name: access04
 *
 * Test Description:
 *  Verify that access() succeeds to check the existance of a file if
 *  search access is permitted on the pathname of the specified file.
 *
 * Expected Result:
 *  access() should return 0 value and the specified file should exist
 *  on the file system.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  access04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TESTDIR		"testdir"
#define TESTFILE	"testdir/testfile"
#define DIR_MODE	S_IRWXU | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

char *TCID = "access04";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* struct buffer for stat(2) */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call access(2) to check the existence of a
		 * file under specified path.
		 */
		TEST(access(TESTFILE, F_OK));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "access(%s, F_OK) failed", TESTFILE);
			continue;
		}

		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Use stat(2) to cross-check the
			 * existance of testfile under
			 * specified path.
			 */
			if (stat(TESTFILE, &stat_buf) < 0) {
				tst_resm(TFAIL, "stat() on %s Failed, errno=%d",
					 TESTFILE, TEST_ERRNO);
			} else {
				tst_resm(TPASS, "Functionality of access(%s, "
					 "F_OK) successful", TESTFILE);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();

 }

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Create a temporary directory and change directory to it.
 *  Create a test directory and a file under test directory.
 *  Modify the mode permissions of testfile.
 */
void setup()
{
	int fd;			/* File handle for testfile */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1)
		tst_resm(TINFO|TERRNO, "setuid(%d) failed", ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	/* Creat a test directory under temporary directory */
	if (mkdir(TESTDIR, DIR_MODE) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir(%s, %#o) failed",
			 TESTDIR, DIR_MODE);

	/* Make sure test directory has search permissions set */
	if (chmod(TESTDIR, DIR_MODE) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "chmod(%s, %#o) failed",
			 TESTDIR, DIR_MODE);

	/* Creat a test file under above directory created */
	fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TESTFILE, FILE_MODE);

	/* Close the testfile created above */
	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "close(%s) failed",
			 TESTFILE);

	/* Change the mode permissions on the testfile */
	if (chmod(TESTFILE, 0) < 0)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "chmod(%s, 0) failed",
			 TESTFILE);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Delete the test directory/file and temporary directory
	 * created in the setup.
	 */
	tst_rmdir();

}
