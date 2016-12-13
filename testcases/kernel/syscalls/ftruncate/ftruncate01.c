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
 * Test Name: ftruncate01
 *
 * Test Description:
 *  Verify that, ftruncate(2) succeeds to truncate a file to a specified
 *  length if the file indicated by file descriptor opened for writing.
 *
 * Expected Result:
 *  ftruncate(2) should return a value 0 and the length of the file after
 *  truncation should be equal to the length it is truncated to.
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
 *  ftruncate01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>

#include "test.h"

#define TESTFILE	"testfile"	/* file under test */
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define BUF_SIZE	256	/* buffer size */
#define FILE_SIZE	1024	/* test file size */
#define TRUNC_LEN	256	/* truncation length */

TCID_DEFINE(ftruncate01);
int TST_TOTAL = 1;		/* Total number of test conditions */
int fildes;			/* file descriptor for test file */

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;
	off_t file_length;	/* test file length */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call ftruncate(2) to truncate a test file to a
		 * specified length.
		 */
		TEST(ftruncate(fildes, TRUNC_LEN));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "ftruncate(%s) failed",
				 TESTFILE);
			continue;
		}
		/*
		 * Get the testfile information using
		 * fstat(2).
		 */
		if (fstat(fildes, &stat_buf) < 0) {
			tst_brkm(TFAIL, cleanup,
				 "stat(2) of %s failed, error:%d",
				 TESTFILE, errno);
		}
		stat_buf.st_mode &= ~S_IFREG;
		file_length = stat_buf.st_size;

		/*
		 * Check for expected size of testfile after
		 * truncate(2) on it.
		 */
		if (file_length != TRUNC_LEN) {
			tst_resm(TFAIL,
				 "%s: Incorrect file size %" PRId64 ", "
				 "Expected %d", TESTFILE,
				 (int64_t) file_length, TRUNC_LEN);
		} else {
			tst_resm(TPASS, "Functionality of ftruncate() "
				 "on %s successful", TESTFILE);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and write some
 *  data into it.
 */
void setup(void)
{
	int i;
	int c, c_total = 0;	/* bytes to be written to file */
	char tst_buff[BUF_SIZE];	/* buffer to hold data */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Fill the test buffer with the known data */
	for (i = 0; i < BUF_SIZE; i++) {
		tst_buff[i] = 'a';
	}

	/* open a file for reading/writing */
	fildes = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
	if (fildes == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) failed",
			 TESTFILE, FILE_MODE);

	/* Write to the file 1k data from the buffer */
	while (c_total < FILE_SIZE) {
		if ((c = write(fildes, tst_buff, sizeof(tst_buff))) <= 0) {
			tst_brkm(TBROK | TERRNO, cleanup, "write(%s) failed",
				 TESTFILE);
		} else {
			c_total += c;
		}
	}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Close the temporary file.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup(void)
{

	/* Close the testfile after writing data into it */
	if (close(fildes) == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "close(%s) failed", TESTFILE);

	tst_rmdir();

}
