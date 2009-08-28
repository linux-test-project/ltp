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
 * Test Name: ftruncate02
 *
 * Test Description:
 *  Verify that, ftruncate(2) succeeds to truncate a file to a certain length,
 *  but the attempt to read past the truncated length will fail.
 *
 * Expected Result:
 *  ftruncate(2) should return a value 0 and the attempt to read past the
 *  truncated length will fail. In case where the file before  truncation was
 *  shorter, the bytes between the old and new should  be all zeroes.
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
 *  ftruncate02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#define TESTFILE	"testfile"	/* file under test */
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define BUF_SIZE	256	/* buffer size */
#define FILE_SIZE	1024	/* test file size */
#define TRUNC_LEN1	256	/* truncation length */
#define TRUNC_LEN2	512	/* truncation length */

TCID_DEFINE(ftruncate02);	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test conditions */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int fd;				/* file descriptor of testfile */
char tst_buff[BUF_SIZE];	/* buffer to hold testfile contents */

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat(2) struct contents */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	off_t file_length2;	/* test file length */
	off_t file_length1;	/* test file length */
	int rbytes, i;		/* bytes read from testfile */
	int read_len = 0;	/* total no. of bytes read from testfile */
	int err_flag = 0;	/* error indicator flag */

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
		 * Call ftruncate(2) to truncate a test file to a
		 * specified length (TRUNC_LEN1).
		 */
		TEST(ftruncate(fd, TRUNC_LEN1));

		/* check return code of ftruncate(2) */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL|TTERRNO, "ftruncate(%s) to size %d failed",
				 TESTFILE, TRUNC_LEN1);
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
			if (fstat(fd, &stat_buf) < 0) {
				tst_brkm(TFAIL, cleanup, "fstat(2) of %s failed"
					 " after 1st truncate, error:%d",
					 TESTFILE, errno);
			}
			stat_buf.st_mode &= ~S_IFREG;
			file_length1 = stat_buf.st_size;

			/*
			 * Set the file pointer of testfile to the
			 * beginning of the file.
			 */
			if (lseek(fd, 0, SEEK_SET) < 0) {
				tst_brkm(TFAIL, cleanup, "lseek(2) on %s failed"
					 " after 1st ftruncate, error:%d",
					 TESTFILE, errno);
			}

			/* Read the testfile from the beginning. */
			while ((rbytes = read(fd, tst_buff,
					      sizeof(tst_buff))) > 0) {
				read_len += rbytes;
			}

			/*
			 * Execute ftruncate(2) again to truncate
			 * testfile to a size TRUNC_LEN2.
			 */
			TEST(ftruncate(fd, TRUNC_LEN2));

			/*
			 * Get the testfile information using
			 * fstat(2)
			 */
			if (fstat(fd, &stat_buf) < 0) {
				tst_brkm(TFAIL, cleanup, "fstat(2) of %s failed"
					 " after 2nd truncate, error:%d",
					 TESTFILE, errno);
			}
			stat_buf.st_mode &= ~S_IFREG;
			file_length2 = stat_buf.st_size;

			/*
			 * Set the file pointer of testfile to the
			 * offset TRUNC_LEN1 of testfile.
			 */
			if (lseek(fd, TRUNC_LEN1, SEEK_SET) < 0) {
				tst_brkm(TFAIL, cleanup, "lseek(2) on %s failed"
					 " after 2nd ftruncate, error:%d",
					 TESTFILE, errno);
			}

			/* Read the testfile contents till EOF */
			while ((rbytes = read(fd, tst_buff,
					      sizeof(tst_buff))) > 0) {
				for (i = 0; i < rbytes; i++) {
					if (tst_buff[i] != 0) {
						err_flag++;
					}
				}
			}

			/*
			 * Check for expected size of testfile after
			 * issuing ftruncate(2) on it. If the ftruncate(2)
			 * to a smaller file passed, then check to see
			 * if file size was increased. If the ftruncate(2)
			 * to a smaller file failed, then don't check.
			 * Both results are allowed according to the SUS.
			 */

			/* check return code of ftruncate(2) */
			if (TEST_RETURN != -1) {
				if ((file_length1 != TRUNC_LEN1) ||
				    (file_length2 != TRUNC_LEN2) ||
				    (read_len != TRUNC_LEN1) ||
				    (err_flag != 0)) {
					tst_resm(TFAIL,
						 "Functionality of ftruncate(2) "
						 "on %s Failed", TESTFILE);
				} else {
					tst_resm(TPASS,
						 "Functionality of ftruncate(2) "
						 "on %s successful", TESTFILE);
				}
			}
			if (TEST_RETURN == -1) {
				if ((file_length1 != TRUNC_LEN1) ||
				    (read_len != TRUNC_LEN1) ||
				    (err_flag != 0)) {
					tst_resm(TFAIL,
						 "Functionality of ftruncate(2) "
						 "on %s Failed", TESTFILE);
				} else {
					tst_resm(TPASS,
						 "Functionality of ftruncate(2) "
						 "on %s successful", TESTFILE);
				}
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
 *  Create a temporary directory and change directory to it.
 *  Create a test file under temporary directory and write some
 *  data into it.
 */
void setup()
{
	int i;			/* counter variable */
	int wbytes;		/* bytes written to testfile */
	int write_len = 0;	/* total no. of bytes written to testfile */

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Fill the test buffer with the known data */
	for (i = 0; i < BUF_SIZE; i++) {
		tst_buff[i] = 'a';
	}
	/* open a file for reading/writing */
	fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
	if (fd == -1)
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %o) failed",
			 TESTFILE, FILE_MODE);

	/* Write to the file 1k data from the buffer */
	while (write_len < FILE_SIZE) {
		if ((wbytes = write(fd, tst_buff, sizeof(tst_buff))) <= 0) {
			tst_brkm(TBROK, cleanup, "write(%s) failed", TESTFILE);
		} else {
			write_len += wbytes;
		}
	}
}				/* End setup() */

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 *  Close the testfile.
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the testfile after writing data into it */
	if (close(fd) == -1)
		tst_brkm(TFAIL|TERRNO, NULL, "close(%s) failed", TESTFILE);

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
