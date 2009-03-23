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
 * Test Name: fchmod03
 *
 * Test Description:
 *  Verify that, fchmod(2) will succeed to change the mode of a file
 *  and set the sticky bit on it if invoked by non-root (uid != 0)
 *  process with the following constraints,
 *	- the process is the owner of the file.
 *	- the effective group ID or one of the supplementary group ID's of the
 *	  process is equal to the group ID of the file.
 *
 * Expected Result:
 *  fchmod() should return value 0 on success and succeeds to change
 *  the mode of specified file, sets sticky bit on it.
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
 *  fchmod03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define FILE_MODE       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define PERMS		01777	/*
				 * Mode permissions of test file with sticky
				 * bit set.
				 */
#define TESTFILE	"testfile"

int fd;				/* file descriptor for test file */
char *TCID = "fchmod03";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup();			/* Main setup function for the test */
void cleanup();			/* Main cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat struct. */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	mode_t file_mode;	/* mode permissions set on testfile */

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

		/*
		 * Call fchmod(2) with specified mode permissions
		 * (to set sticky bit) on testfile.
		 */
		TEST(fchmod(fd, PERMS));

		/* check return code of fchmod(2) */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "fchmod(%d, %#o) Failed, errno=%d : %s",
				 fd, PERMS, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Get the file information using
			 * fstat(2).
			 */
			if (fstat(fd, &stat_buf) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "fstat(2) of %s failed, errno:%d",
					 TESTFILE, TEST_ERRNO);
			}
			file_mode = stat_buf.st_mode;

			/* Verify STICKY BIT set on testfile */
			if ((file_mode & PERMS) != PERMS) {
				tst_resm(TFAIL, "%s: Incorrect modes 0%3o, "
					 "Expected 0777", TESTFILE, file_mode);
			} else {
				tst_resm(TPASS, "Functionality of fchmod(%d, "
					 "%#o) successful", fd, PERMS);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and cd to it.
 *  Create a testfile under test directory.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/*
	 * Create a test file under temporary directory with specified
	 * mode permissios and set the ownership of the test file to the
	 * uid/gid of guest user.
	 */
	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) Failed, errno=%d : %s",
			 TESTFILE, FILE_MODE, errno, strerror(errno));
	}
}				/* End setup() */

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 *  Close the testfile created in the setup.
 *  Delete the testfile and temporary directory created in setup().
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the testfile created/opened in the setup */
	if (close(fd) == -1) {
		tst_brkm(TBROK, NULL, "close(%s) Failed, errno=%d : %s",
			 TESTFILE, errno, strerror(errno));
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
