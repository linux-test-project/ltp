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
 * Test Name: pread02
 *
 * Test Description:
 *  Verify that,
 *   1) pread() fails when attempted to read from an unnamed pipe.
 *   2) pread() fails if the specified offset position was invalid.
 *
 * Expected Result:
 *  1) pread() should return -1 and set errno to ESPIPE.
 *  2) pread() should return -1 and set errno to EINVAL.
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
 *  pread02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
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
#include "safe_macros.h"

#define TEMPFILE	"pread_file"
#define K1              1024
#define NBUFS           4

char *TCID = "pread02";
int TST_TOTAL = 2;

char *write_buf[NBUFS];		/* buffer to hold data to be written */
char *read_buf[NBUFS];		/* buffer to hold data read from file */
int pfd[2];			/* pair of file descriptors */
int fd1;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
int setup1();			/* setup function for test #1 */
int setup2();			/* setup function for test #2 */
int no_setup();
void init_buffers();		/* function to initialize/allocate buffers */

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
	0, 0, 0, NULL, 0, no_setup}
};

int main(int ac, char **av)
{
	int lc;
	int i;
	int fildes;		/* file descriptor of test file */
	size_t nbytes;		/* no. of bytes to be written */
	off_t offset;		/* offset position in the specified file */
	char *test_desc;	/* test specific error message */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* loop through the test cases */
		for (i = 0; Test_cases[i].desc != NULL; i++) {
			fildes = Test_cases[i].fd;
			test_desc = Test_cases[i].desc;
			nbytes = Test_cases[i].nb;
			offset = Test_cases[i].offst;

			if (fildes == 1) {
				fildes = pfd[0];
			} else if (fildes == 2) {
				fildes = fd1;
			}

			/*
			 * Call pread() with the specified file descriptor,
			 * no. of bytes to be read from specified offset.
			 * and verify that call should fail with appropriate
			 * errno set.
			 */
			TEST(pread(fildes, read_buf[0], nbytes, offset));

			/* Check for the return code of pread() */
			if (TEST_RETURN != -1) {
				tst_brkm(TFAIL, cleanup, "pread() returned "
					 "%ld, expected -1, errno:%d",
					 TEST_RETURN, Test_cases[i].exp_errno);
			}

			/*
			 * Verify whether expected errno is set.
			 */
			if (TEST_ERRNO == Test_cases[i].exp_errno) {
				tst_resm(TPASS, "pread() fails, %s, errno:%d",
					 test_desc, TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "pread() fails, %s, unexpected "
					 "errno:%d, expected:%d", test_desc,
					 TEST_ERRNO, Test_cases[i].exp_errno);
			}
		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *           Initialize/allocate write buffer.
 *           Call individual setup function.
 */
void setup(void)
{
	int i;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Allocate/Initialize the read/write buffer with known data */
	init_buffers();

	/* Call individual setup functions */
	for (i = 0; Test_cases[i].desc != NULL; i++) {
		Test_cases[i].setupfunc();
	}
}

/*
 * no_setup() - This function simply returns.
 */
int no_setup(void)
{
	return 0;
}

/*
 * setup1() - setup function for a test condition for which pread()
 *            returns -ve value and sets errno to ESPIPE.
 *
 *  Create an unnamed pipe using pipe().
 *  Write some known data to the write end of the pipe.
 *  return 0.
 */
int setup1(void)
{
	/* Create a pair of unnamed pipe */
	SAFE_PIPE(cleanup, pfd);

	/* Write known data (0's) of K1 bytes */
	if (write(pfd[1], write_buf[0], K1) != K1) {
		tst_brkm(TBROK, cleanup, "write to pipe failed: errno=%d : %s",
			 errno, strerror(errno));
	}

	return 0;
}

/*
 * setup2 - setup function for a test condition for which pread()
 *          returns -ve value and sets errno to EINVAL.
 *
 *  Create a temporary directory and a file under it.
 *  return 0.
 */
int setup2(void)
{

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
 *  Allocate read/write buffer.
 *  Fill the write buffer with the following data like,
 *    write_buf[0] has 0's, write_buf[1] has 1's, write_buf[2] has 2's
 *    write_buf[3] has 3's.
 */
void init_buffers(void)
{
	int count;		/* counter variable for loop */

	/* Allocate and Initialize write buffer with known data */
	for (count = 0; count < NBUFS; count++) {
		write_buf[count] = malloc(K1);
		read_buf[count] = malloc(K1);

		if ((write_buf[count] == NULL) || (read_buf[count] == NULL)) {
			tst_brkm(TBROK, NULL,
				 "malloc() failed on read/write buffers");
		}
		memset(write_buf[count], count, K1);
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *  Deallocate the memory allocated to read/write buffers.
 *  Close the temporary file.
 *  Remove the temporary directory created.
 */
void cleanup(void)
{
	int count;

	/* Free the memory allocated for the read/write buffer */
	for (count = 0; count < NBUFS; count++) {
		free(write_buf[count]);
		free(read_buf[count]);
	}

	/* Close the temporary file created in setup2 */
	SAFE_CLOSE(NULL, fd1);

	tst_rmdir();

}
