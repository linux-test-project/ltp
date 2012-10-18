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
 * Test Name : readlink01
 *
 * Test Description :
 *  Verify that, readlink will succeed to read the contents of the symbolic
 *  link created the process.
 *
 * Expected Result:
 *  readlink() should return the contents of symbolic link path in the buffer
 *  on success.
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
 *   	Issue a FAIL message.
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
 *  readlink01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be run by 'non-super-user' only.
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TESTFILE	"testfile"
#define SYMFILE		"slink_file"
#define FILE_MODE       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define MAX_SIZE	256

char *TCID = "readlink01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int exp_val;			/* strlen of testfile */

void setup();			/* Setup function for the test */
void cleanup();			/* Cleanup function for the test */

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int main(int ac, char **av)
{
	char buffer[MAX_SIZE];	/* temporary buffer to hold symlink contents */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call readlink(2) to read the contents of
		 * symlink into a buffer.
		 */
		TEST(readlink(SYMFILE, buffer, sizeof(buffer)));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "readlink() on %s failed, errno=%d : %s",
				 SYMFILE, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Compare the return value of readlink()
			 * with the expected value which is the
			 * strlen() of testfile.
			 */
			if (TEST_RETURN == exp_val) {
				/* Check for the contents of buffer */
				if (memcmp(buffer, TESTFILE, exp_val) != 0) {
					tst_resm(TFAIL, "Pathname %s and buffer"
						 " contents %s differ",
						 TESTFILE, buffer);
				} else {
					tst_resm(TPASS, "readlink() "
						 "functionality on '%s' is "
						 "correct", SYMFILE);
				}
			} else {
				tst_resm(TFAIL, "readlink() return value %ld "
					 "does't match, Expected %d",
					 TEST_RETURN, exp_val);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and close it
 *  Create a symbolic link of testfile.
 */
void setup()
{
	int fd;			/* file handle for testfile */

	tst_require_root(NULL);

	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, cleanup, "getpwname(nobody_uid) failed ");
	}
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup, "seteuid to nobody failed");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TESTFILE, FILE_MODE);
	}

	if (close(fd) == -1) {
		tst_resm(TWARN|TERRNO, "close(%s) failed", TESTFILE);
	}

	/* Create a symlink of testfile under temporary directory */
	if (symlink(TESTFILE, SYMFILE) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "symlink(%s, %s) failed",
		    TESTFILE, SYMFILE);
	}

	/* Get the strlen of testfile */
	exp_val = strlen(TESTFILE);
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

	tst_rmdir();

}
