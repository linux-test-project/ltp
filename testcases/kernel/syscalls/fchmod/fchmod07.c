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
 * Test Name: fchmod07
 *
 * Test Description:
 *  Verify that, fchmod(2) succeeds when used to change the mode permissions
 *  of a file specified by file descriptor.
 *
 * Expected Result:
 *  fchmod(2) should return 0 and the mode permissions set on file should match
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
 *  fchmod07 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TESTFILE	"testfile"

int fd;				/* file descriptor for testfile */
char *TCID = "fchmod07";
int TST_TOTAL = 8;		/* Total number of test conditions */

int Modes[] = { 0, 07, 070, 0700, 0777, 02777, 04777, 06777 };

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;
	int ind;		/* counter variable for chmod(2) tests */
	int mode;		/* file mode permission */

	TST_TOTAL = sizeof(Modes) / sizeof(int);

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			mode = Modes[ind];

			/*
			 * Call fchmod(2) with different mode permission
			 * bits to set it for "testfile".
			 */
			TEST(fchmod(fd, mode));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "fchmod(%d, %#o) Failed, "
					 "errno=%d : %s", fd, mode, TEST_ERRNO,
					 strerror(TEST_ERRNO));
				continue;
			}
			/*
			 * Get the testfile information using
			 * fstat(2).
			 */
			if (fstat(fd, &stat_buf) < 0) {
				tst_brkm(TFAIL, cleanup,
					 "fstat(2) of "
					 "%s failed, errno:%d",
					 TESTFILE, TEST_ERRNO);
			}
			stat_buf.st_mode &= ~S_IFREG;

			/*
			 * Check for expected mode permissions
			 * on testfile.
			 */
			if (stat_buf.st_mode == mode) {
				tst_resm(TPASS,
					 "Functionality of "
					 "fchmod(%d, %#o) successful",
					 fd, mode);
			} else {
				tst_resm(TFAIL, "%s: Incorrect modes "
					 "0%03o, Expected 0%03o",
					 TESTFILE, stat_buf.st_mode,
					 mode);
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
 *  Create a test file under temporary directory.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) Failed, errno=%d : %s",
			 TESTFILE, FILE_MODE, errno, strerror(errno));
	}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Close the testfile created in the setup.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup(void)
{

	/* Close the TESTFILE opened in the setup() */
	SAFE_CLOSE(NULL, fd);

	tst_rmdir();

}
