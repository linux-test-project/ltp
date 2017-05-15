/*
 *
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: pread03
 *
 * Test Description:
 *  Verify that,
 *   1) pread() fails when fd refers to a directory.
 *
 *
 * Expected Result:
 *   1) pread() should return -1 and set errno to EISDIR.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *   Create a temporary directory.
 *   Get the currect directory name
 *   Open temporary directory
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
 *  pread03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	04/2002 Ported by André Merlier
 *
 * RESTRICTIONS:
 *  None.
 */

#define _XOPEN_SOURCE 500

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>

#include "test.h"

#define PREAD_TEMPDIR	"test"
#define K1              2048
#define NBUFS           1

char *TCID = "pread03";
int TST_TOTAL = 1;

char *read_buf[NBUFS];		/* buffer to hold data read from file */
int fd1;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void init_buffers();		/* function to initialize/allocate buffers */

int main(int ac, char **av)
{
	int lc;
	size_t nbytes;		/* no. of bytes to be written */
	off_t offset;		/* offset position in the specified file */
	char *test_desc;	/* test specific error message */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		test_desc = "EISDIR";
		nbytes = K1;
		offset = 20;

		TEST(pread(fd1, read_buf[0], nbytes, offset));

		/* Check for the return code of pread() */
		if (TEST_RETURN != -1) {
			tst_brkm(TFAIL, cleanup, "pread() returned "
				 "%ld, expected -1, errno:%d\n",
				 TEST_RETURN, EISDIR);
		}

		/*
		 * Verify whether expected errno is set.
		 */
		if (TEST_ERRNO == EISDIR) {
			tst_resm(TPASS,
				 "pread() fails with expected error EISDIR errno:%d",
				 TEST_ERRNO);
		} else {
			tst_resm(TFAIL, "pread() fails, %s, unexpected "
				 "errno:%d, expected:%d\n", test_desc,
				 TEST_ERRNO, EISDIR);
		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *           create temporary directory and open it
 */
void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Allocate the read buffer */
	init_buffers();

	tst_tmpdir();

	/*
	 * create a temporary directory
	 */
	if (mkdir(PREAD_TEMPDIR, 0777) != 0) {
		tst_resm(TFAIL, "mkdir() failed to create" " test directory");
		exit(1);

	}

	/* open temporary directory used for test */
	if ((fd1 = open(PREAD_TEMPDIR, O_RDONLY)) < 0) {
		tst_brkm(TBROK, cleanup, "open() on %s Failed, errno=%d : %s",
			 PREAD_TEMPDIR, errno, strerror(errno));
	}
}

/*
 * init_buffers() - allocate/Initialize write_buf array.
 *
 *  Allocate read buffer.
 */
void init_buffers(void)
{
	int count;		/* counter variable for loop */

	/* Allocate and Initialize read buffer */
	for (count = 0; count < NBUFS; count++) {
		read_buf[count] = malloc(K1);

		if (read_buf[count] == NULL) {
			tst_brkm(TBROK, NULL,
				 "malloc() failed on read buffers");
		}
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *  Close/Remove the temporary directory created.
 */
void cleanup(void)
{
	int count;

	/* Free the memory allocated for the read buffer */
	for (count = 0; count < NBUFS; count++) {
		free(read_buf[count]);
	}

	/* delete the test directory created in setup() */
	tst_rmdir();

}
