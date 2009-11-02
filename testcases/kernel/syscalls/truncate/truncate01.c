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
 * Test Name: truncate01
 *
 * Test Description:
 *  Verify that, truncate(2) succeeds to truncate a file to a specified
 *  length.
 *
 * Expected Result:
 *  truncate(2) should return a value 0 and the length of the file after
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
 *  truncate01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
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
 *  This test should be run by 'non-super-user' only.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>

#include "test.h"
#include "usctest.h"

#define TESTFILE	"testfile"	/* file under test */
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define BUF_SIZE	256	/* buffer size */
#define FILE_SIZE	1024	/* test file size */
#define TRUNC_LEN	256	/* truncation length */

TCID_DEFINE(truncate01);	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test conditions */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { 0 };

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	off_t file_length;	/* test file length */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call truncate(2) to truncate a test file to a
		 * specified length.
		 */
		TEST(truncate(TESTFILE, TRUNC_LEN));

		/* check return code of truncate(2) */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "truncate(%s, %d) Failed, errno=%d : %s",
				 TESTFILE, TRUNC_LEN, TEST_ERRNO,
				 strerror(TEST_ERRNO));
		} else {
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
						 "%s failed, error:%d",
						 TESTFILE, errno);
				 /*NOTREACHED*/}
				stat_buf.st_mode &= ~S_IFREG;
				file_length = stat_buf.st_size;

				/*
				 * Check for expected size of testfile after
				 * truncate(2) on it.
				 */
				if (file_length != TRUNC_LEN) {
					tst_resm(TFAIL, "%s: Incorrect file "
						 "size %"PRId64", Expected %d",
						 TESTFILE, (int64_t)file_length,
						 TRUNC_LEN);
				} else {
					tst_resm(TPASS, "Functionality of "
						 "truncate(%s, %d) successful",
						 TESTFILE, TRUNC_LEN);
				}
			} else {
				tst_resm(TPASS, "%s call succeeded", TCID);
			}
		}
		Tst_count++;	/* incr TEST_LOOP counter */
	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();
	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Create a temporary directory and change directory to it.
 *  Fill the buffer with some arbitrary data to be written to a file.
 *  Create a test file under temporary directory and close it
 *  write arbitrary data into testfile.
 */
void setup()
{
	int fd, i;		/* file handler for testfile */
	int c, c_total = 0;	/* no. bytes to be written to file */
	char tst_buff[BUF_SIZE];	/* buffer to hold data */

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Fill the test buffer with the known data */
	for (i = 0; i < BUF_SIZE; i++) {
		tst_buff[i] = 'a';
	}

	/* Creat a testfile under temporary directory */
	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) Failed, errno=%d : %s",
			 TESTFILE, FILE_MODE, errno, strerror(errno));
	 /*NOTREACHED*/}

	/* Write to the file 1k data from the buffer */
	while (c_total < FILE_SIZE) {
		if ((c = write(fd, tst_buff, sizeof(tst_buff))) <= 0) {
			tst_brkm(TBROK, cleanup,
				 "write(2) on %s Failed, errno=%d : %s",
				 TESTFILE, errno, strerror(errno));
		 /*NOTREACHED*/} else {
			c_total += c;
		}
	}

	/* Close the testfile after writing data into it */
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup,
			 "close(%s) Failed, errno=%d : %s",
			 TESTFILE, errno, strerror(errno));
	 /*NOTREACHED*/}
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
