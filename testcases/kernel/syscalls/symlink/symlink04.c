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
 * Test Name : symlink04
 *
 * Test Description :
 *  Verify that, symlink will succeed to creat a symbolic link of an existing
 *  object name path.
 *
 * Expected Result:
 *  symlink() should return value 0 on success and symbolic link of an
 *  existing object should be created.
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
 *  symlink04 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 *  None.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include "test.h"
#include "safe_macros.h"

#define  TESTFILE	"testfile"
#define  SYMFILE	"slink_file"
#define FILE_MODE       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

char *TCID = "symlink04";
int TST_TOTAL = 1;

void setup();
void cleanup();

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat structure buffer */
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call symlink(2) to create a symlink of
		 * testfile.
		 */
		TEST(symlink(TESTFILE, SYMFILE));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "symlink(%s, %s) Failed, errno=%d : %s",
				 TESTFILE, SYMFILE, TEST_ERRNO,
				 strerror(TEST_ERRNO));
		} else {
			/*
			 * Get the symlink file status information
			 * using lstat(2).
			 */
			if (lstat(SYMFILE, &stat_buf) < 0) {
				tst_brkm(TFAIL, cleanup, "lstat(2) of "
					 "%s failed, error:%d", SYMFILE,
					 errno);
			}

			/* Check if the st_mode contains a link  */
			if (!S_ISLNK(stat_buf.st_mode)) {
				tst_resm(TFAIL,
					 "symlink of %s doesn't exist",
					 TESTFILE);
			} else {
				tst_resm(TPASS, "symlink(%s, %s) "
					 "functionality successful",
					 TESTFILE, SYMFILE);
			}
		}

		/* Unlink the symlink file for next loop */
		SAFE_UNLINK(cleanup, SYMFILE);
		tst_count++;	/* incr TEST_LOOP counter */
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
	int fd;			/* file handle for testfile */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	tst_tmpdir();

	/* creat/open a testfile */
	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) Failed, errno=%d : %s",
			 TESTFILE, FILE_MODE, errno, strerror(errno));
	}

	/* Close the temporary file created above */
	if (close(fd) == -1) {
		tst_resm(TBROK, "close(%s) Failed, errno=%d : %s",
			 TESTFILE, errno, strerror(errno));
	}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup(void)
{

	tst_rmdir();

}
