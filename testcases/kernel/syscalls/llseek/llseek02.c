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
 * Test Name: llseek02
 * Note that glibc exports the llseek syscall as lseek64.
 *
 * Test Description:
 *  Verify that,
 *  1. llseek() returns -1 and sets errno to EINVAL, if the 'Whence' argument
 *     is not a proper value.
 *  2. llseek() returns -1 and sets errno to EBADF, if the file handle of
 *     the specified file is not valid.
 *
 * Expected Result:
 *  llseek() should fail with return value -1 and set expected errno.
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
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  llseek02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
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

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"

#define TEMP_FILE1	"tmp_file1"
#define TEMP_FILE2	"tmp_file2"
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define SEEK_TOP	10

char *TCID = "llseek02";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int no_setup();
int setup1();			/* setup function to test llseek() for EINVAL */
int setup2();			/* setup function to test llseek() for EBADF */

int fd1;			/* file handle for testfile1  */
int fd2;			/* file handle for testfile2  */

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	int fd;
	int Whence;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	1, SEEK_TOP, "'whence' argument is not valid", EINVAL, setup1}, {
	2, SEEK_SET, "'fd' is not an open file descriptor", EBADF, setup2},
	{
	0, 0, NULL, 0, no_setup}
};

int exp_enos[] = { EINVAL, EBADF, 0 };

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int fildes;		/* file handle for testfile */
	int whence;		/* position of file handle in the file */
	char *test_desc;	/* test specific error message */
	int ind;		/* counter to test different test conditions */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* set up expected error numbers */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			fildes = Test_cases[ind].fd;
			test_desc = Test_cases[ind].desc;
			whence = Test_cases[ind].Whence;

			/* Assign the 'fd' values appropriatly */
			if (fildes == 1) {
				fildes = fd1;
			} else {
				fildes = fd2;
			}

			/*
			 * Invoke llseek(2) to test different test conditions.
			 * Verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(lseek64(fildes, (loff_t) 0, whence));

			/* check return code of llseek(2) */
			if (TEST_RETURN != (loff_t) - 1) {
				tst_resm(TFAIL, "llseek() returned %ld, expected"
					 " -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
				continue;
			}
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == Test_cases[ind].exp_errno) {
				tst_resm(TPASS, "llseek() fails, %s, errno:%d",
					 test_desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "llseek() fails, %s, errno:%d, "
					 "expected errno:%d", test_desc,
					 TEST_ERRNO, Test_cases[ind].exp_errno);
			}
		}
	}

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *           Create a temporary directory and change directory to it.
 *           Invoke individual test setup functions according to the order
 *           set in test struct. definition.
 */
void setup()
{
	int ind;		/* counter for test setup function */

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}

/*
 * no_setup() - This is a dummy function which simply returns 0.
 */
int no_setup()
{
	return 0;
}

/*
 * setup1() - setup function for a test condition for which llseek(2)
 *            returns -1 and sets errno to EINVAL.
 *            Creat a temporary file for reading/writing and write some data
 *            into it.
 *            This function returns 0 on success.
 */
int setup1()
{
	char write_buff[BUFSIZ];	/* buffer to hold data */

	/* Get the data to be written to temporary file */
	strcpy(write_buff, "abcdefg");

	/* Creat/open a temporary file under above directory */
	if ((fd1 = open(TEMP_FILE1, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) Failed, errno=%d :%s",
			 TEMP_FILE1, FILE_MODE, errno, strerror(errno));
	}

	/* Write data into temporary file */
	if (write(fd1, write_buff, sizeof(write_buff)) <= 0) {
		tst_brkm(TBROK, cleanup, "write(2) on %s Failed, errno=%d : %s",
			 TEMP_FILE1, errno, strerror(errno));
	}

	return 0;
}

/*
 * setup2() - setup function for a test condition for which llseek(2)
 *            returns -1 and sets errno to EBADF.
 *            Creat a temporary file for reading/writing and close it.
 *            This function returns 0 on success.
 */
int setup2()
{
	/* Creat/open a temporary file under above directory */
	if ((fd2 = open(TEMP_FILE2, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) Failed, errno=%d :%s",
			 TEMP_FILE2, FILE_MODE, errno, strerror(errno));
	}

	/* Close the temporary file created above */
	if (close(fd2) < 0) {
		tst_brkm(TBROK, cleanup, "close(%s) Failed, errno=%d : %s:",
			 TEMP_FILE2, errno, strerror(errno));
	}

	return 0;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *             Close the temporary file.
 *             Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the temporary file(s) created in setup1/setup2 */
	if (close(fd1) < 0) {
		tst_brkm(TFAIL, NULL, "close(%s) Failed, errno=%d : %s:",
			 TEMP_FILE1, errno, strerror(errno));
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
