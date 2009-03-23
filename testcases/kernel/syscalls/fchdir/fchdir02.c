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
 * NAME
 *	fchdir02.c
 *
 * DESCRIPTION
 *	fchdir02 - try to cd into a bad directory (bad fd).
 *
 * CALLS
 *	fchdir()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	call fchdir() with an invalid file descriptor
 *	check the errno value
 *	  issue a PASS message if we get EBADF - errno 9
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  fchdir02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

void cleanup(void);
void setup(void);

char *TCID = "fchdir02";
int TST_TOTAL = 1;
extern int Tst_count;

int exp_enos[] = { 9, 0 };	/* 0 terminated list of expected errnos */

int main(int ac, char **av)
{
	const int bad_fd = -5;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * Look for a failure by using an invalid number for fd
		 */

		TEST(fchdir(bad_fd));

		if (TEST_RETURN != -1) {
			tst_brkm(TFAIL, cleanup, "call succeeded with bad "
				 "file descriptor");
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case EBADF:
			tst_resm(TPASS, "expected failure - errno = %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_brkm(TFAIL, cleanup, "call failed with an "
				 "unexpected error - %d : %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}
	}

	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* create a test directory and cd into it */
	tst_tmpdir();

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* remove the test directory */
	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
