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
 * Test Name: pwrite02
 *
 * Test Description:
 *  Verify that,
 *   1) pwrite() fails when attempted to write to an unnamed pipe.
 *   2) pwrite() fails if the specified offset position was invalid.
 *
 * Expected Result:
 *  1) pwrite() should return -1 and set errno to ESPIPE.
 *  2) pwrite() should return -1 and set errno to EINVAL.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create a temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *      if errno set == expected errno
 *              Issue sys call fails with expected return value and errno.
 *      Otherwise,
 *              Issue sys call fails with unexpected errno.
 *   Otherwise,
 *      Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  pwrite02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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

#define _XOPEN_SOURCE 500

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"pwrite_file"
#define K1              1024
#define NBUFS           4

char *TCID = "pwrite02";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char *write_buf[NBUFS];		/* buffer to hold data to be written */
int pfd[2];			/* pair of file descriptors */
int fd1;			/* file descriptor of temporary file */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
int setup1();			/* setup function for test #1 */
int setup2();			/* setup function for test #2 */
int no_setup();
void init_buffers();		/* function to initialize/allocate buffers */

int exp_enos[] = { ESPIPE, EINVAL, EBADF, 0 };

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	int fd;
	size_t nb;
	off_t offst;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	1, K1, 0, "file descriptor is a PIPE or FIFO", ESPIPE, setup1}, {
	2, K1, -1, "specified offset is -ve or invalid", EINVAL, setup2}, {
	3, K1, 0, "file descriptor is bad", EBADF, no_setup}, {
	0, 0, 0, NULL, 0, no_setup}
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;			/* counter to test different test conditions */
	int fildes;		/* file descriptor of test file */
	size_t nbytes;		/* no. of bytes to be written */
	off_t offset;		/* offset position in the specified file */
	char *test_desc;	/* test specific error message */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Perform global setup for test */
	setup();

	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; Test_cases[i].desc != NULL; i++) {
			fildes = Test_cases[i].fd;
			test_desc = Test_cases[i].desc;
			nbytes = Test_cases[i].nb;
			offset = Test_cases[i].offst;

			if (fildes == 1) {
				fildes = pfd[1];
			} else if (fildes == 2) {
				fildes = fd1;
			} else {
				fildes = -1;
			}

			/*
			 * Call pwrite() with the specified file descriptor,
			 * no. of bytes to be written at specified offset.
			 * and verify that call should fail with appropriate
			 * errno. set.
			 */
			TEST(pwrite(fildes, write_buf[0], nbytes, offset));

			/* Check for the return code of pwrite() */
			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "pwrite() returned %ld, "
					 "expected -1, errno:%d",
					 TEST_RETURN, Test_cases[i].exp_errno);
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			/*
			 * Verify whether expected errno is set.
			 */
			if (TEST_ERRNO == Test_cases[i].exp_errno) {
				tst_resm(TPASS, "%s, errno:%d",
					 test_desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "%s, unexpected"
					 " errno:%d, expected:%d", test_desc,
					 TEST_ERRNO, Test_cases[i].exp_errno);
			}
		}		/* End of TEST CASE LOOPING */
	}			/* End of TEST_LOOPING. */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
}				/* End main */

/*
 * sighandler - handle SIGXFSZ
 *
 * This is here to start looking at a failure in test case #2.  This
 * test case passes on a machine running RedHat 6.2 but it will fail
 * on a machine running RedHat 7.1.
 */
void sighandler(sig)
{
	if (sig != SIGXFSZ) {
		printf("wrong signal\n");
	} else {
		printf("caught SIGXFSZ\n");
	}
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Initialize/allocate write buffer.
 *  Call individual setup function.
 */
void setup()
{
	int i;			/* counter for setup functions */

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* see the comment in the sighandler() function */
	/* call signal() to trap the signal generated */
	if (signal(SIGXFSZ, sighandler) == SIG_ERR) {
		tst_brkm(TBROK, cleanup, "signal() failed");
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Allocate/Initialize the write buffer with known data */
	init_buffers();

	/* Call individual setup functions */
	for (i = 0; Test_cases[i].desc != NULL; i++) {
		Test_cases[i].setupfunc();
	}
}

/*
 * no_setup() - This function simply returns.
 */
int no_setup()
{
	return 0;
}

/*
 * setup1() - setup function for a test condition for which pwrite()
 *	      returns -ve value and sets errno to ESPIPE.
 *
 *  Create an unnamed pipe using pipe().
 *  return 0.
 */
int setup1()
{
	/* Create an unnamed pipe */
	if (pipe(pfd) < 0) {
		tst_brkm(TBROK, cleanup, "pipe() failed, error:%d", errno);
	}

	return 0;
}

/*
 * setup2 - setup function for a test condition for which pwrite()
 * 	    returns -ve value and sets errno to EINVAL.
 *
 *  Create a temporary directory and a file under it.
 *  return 0.
 */
int setup2()
{
	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fd1 = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	return 0;
}

/*
 * init_buffers() - allocate/Initialize write_buf array.
 *
 *  Allocate write buffer.
 *  Fill the write buffer with the following data like,
 *    write_buf[0] has 0's, write_buf[1] has 1's, write_buf[2] has 2's
 *    write_buf[3] has 3's.
 */
void init_buffers()
{
	int count;		/* counter variable for loop */

	/* Allocate and Initialize write buffer with known data */
	for (count = 0; count < NBUFS; count++) {
		write_buf[count] = (char *)malloc(K1);

		if (write_buf[count] == NULL) {
			tst_brkm(TBROK, tst_exit,
				 "malloc() failed on write buffer");
		}
		memset(write_buf[count], count, K1);
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 * Deallocate the memory allocated to write buffer.
 * Close the temporary file.
 * Remove the temporary directory created.
 */
void cleanup()
{
	int count;		/* index for the loop */

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Free the memory allocated for the write buffer */
	for (count = 0; count < NBUFS; count++) {
		free(write_buf[count]);
	}

	/* Close the temporary file created in setup2 */
	if (close(fd1) < 0) {
		tst_brkm(TBROK, NULL,
			 "close() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
