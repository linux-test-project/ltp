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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
#include "usctest.h"

#define PREAD_TEMPDIR	"test"
#define K1              2048
#define NBUFS           1

char *TCID = "pread03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char *read_buf[NBUFS];		/* buffer to hold data read from file */
char test_dir[100];
int fd1;			/* file descriptor of temporary file */
int exp_enos[] = { EISDIR, 0 };

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void init_buffers();		/* function to initialize/allocate buffers */

int main(int ac, char **av)
{
	int lc;
	char *msg;		/* message returned from parse_opts */
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

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

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

		TEST_ERROR_LOG(TEST_ERRNO);

		/*
		 * Verify whether expected errno is set.
		 */
		if (TEST_ERRNO == exp_enos[0]) {
			tst_resm(TPASS,
				 "pread() fails with expected error EISDIR errno:%d",
				 TEST_ERRNO);
		} else {
			tst_resm(TFAIL, "pread() fails, %s, unexpected "
				 "errno:%d, expected:%d\n", test_desc,
				 TEST_ERRNO, exp_enos[0]);
		}
	}

	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *           create temporary directory and open it
 */
void setup()
{
	char *cur_dir = NULL;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Allocate the read buffer */
	init_buffers();

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* get the currect directory name */
	if ((cur_dir = getcwd(cur_dir, 0)) == NULL) {
		tst_brkm(TBROK, cleanup, "Couldn't get current directory name");
	}

	sprintf(test_dir, "%s.%d", cur_dir, getpid());

	/*
	 * create a temporary directory
	 */
	if (mkdir(PREAD_TEMPDIR, 0777) != 0) {
		tst_resm(TFAIL, "mkdir() failed to create" " test directory");
		exit(1);
		/* NOTREACHED */
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
void init_buffers()
{
	int count;		/* counter variable for loop */

	/* Allocate and Initialize read buffer */
	for (count = 0; count < NBUFS; count++) {
		read_buf[count] = (char *)malloc(K1);

		if (read_buf[count] == NULL) {
			tst_brkm(TBROK, tst_exit,
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
void cleanup()
{
	int count;		/* index for the loop */

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Free the memory allocated for the read buffer */
	for (count = 0; count < NBUFS; count++) {
		free(read_buf[count]);
	}

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
