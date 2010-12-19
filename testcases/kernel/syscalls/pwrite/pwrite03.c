/*
 *   Copyright (C) Bull S.A. 2001
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
 * Test Name: pwrite03
 *
 * Test Description:
 *  Verify that,
 *      pwrite(2) fails when attempted to write with buf outside accessible
 *      address space.
 *
 * Expected Result:
 *      pwrite() should return -1 and set errno to EFAULT.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create a temporary directory/file.
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
 *  pwrite03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	04/2002 Ported by André Merlier
 *
 * RESTRICTIONS:
 *  None.
 */

#define _XOPEN_SOURCE 500

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"pwrite_file"
#define NBUFS           1

char *TCID = "pwrite03";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

char *write_buf[NBUFS];		/* buffer to hold data to be written */
int fd1;			/* file descriptor of temporary file */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void init_buffers();		/* function to initialize/allocate buffers */

int exp_enos[] = { EFAULT, 0 };

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	size_t nbytes;		/* no. of bytes to be written */
	off_t offset;		/* offset position in the specified file */
	char *test_desc;	/* test specific error message */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		test_desc = "EFAULT";
		nbytes = 1024;
		offset = 1;
		write_buf[0] = sbrk(0);

		/*
		 * Call pwrite() with the specified file descriptor,
		 * no. of bytes to be written at specified offset.
		 * and verify that call should fail with appropriate
		 * errno. set.
		 */
		TEST(pwrite(fd1, write_buf[0], nbytes, offset));

		/* Check for the return code of pwrite() */
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "pwrite() returned %ld, "
				 "expected -1, errno:%d",
				 TEST_RETURN, exp_enos[0]);
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		/*
		 * Verify whether expected errno is set.
		 */
		if (TEST_ERRNO == exp_enos[0]) {
			tst_resm(TPASS, "pwrite() fails with expected "
				 "error EFAULT errno:%d", TEST_ERRNO);
		} else {
			tst_resm(TFAIL, "pread() fails, %s, unexpected "
				 "errno:%d, expected:%d\n", test_desc,
				 TEST_ERRNO, exp_enos[0]);
		}

	}

	cleanup();
	tst_exit();
	tst_exit();

}

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Initialize/allocate write buffer.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fd1 = open(TEMPFILE, O_RDWR | O_CREAT, 0777)) < 0) {
		tst_brkm(TBROK, cleanup, "open() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 * Close the temporary file.
 * Remove the temporary directory created.
 */
void cleanup()
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Close the temporary file created in setup2 */
	if (close(fd1) < 0) {
		tst_brkm(TBROK, NULL,
			 "close() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	tst_rmdir();

}