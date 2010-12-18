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
 * Test Name: mknod09
 *
 * Test Description:
 *  Verify that, mknod() fails with -1 and sets errno to EINVAL if the mode is
 *  different than a normal file, device special file or FIFO.
 *
 * Expected Result:
 *  mknod() should fail with return value -1 and sets expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Check id is super/root
 *   Pause for SIGUSR1 if option specified.
 *   Create temporary directory.
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
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  mknod09 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	05/2002 Ported by André Merlier
 *
 * RESTRICTIONS:
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include <errno.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

#define MODE_RWX	S_IFMT	/* mode different from those expected */
#define TNODE		"tnode"	/*pathname */

char *TCID = "mknod09";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
int exp_enos[] = { EINVAL, 0 };

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *test_desc;	/* test specific error message */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		test_desc = "EINVAL";

		Tst_count = 0;

		/*
		 * Call mknod(2) to test condition.
		 * verify that it fails with -1 return value and
		 * sets appropriate errno.
		 */
		TEST(mknod(TNODE, MODE_RWX, 0));

		/* Check return code from mknod(2) */
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "mknod() returned %ld,"
				 "expected -1, errno=%d", TEST_RETURN,
				 exp_enos[0]);
		} else {
			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == exp_enos[0]) {
				tst_resm(TPASS, "mknod() fails with expected "
					 "error EINVAL errno:%d", TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "mknod() fails, %s, "
					 "errno=%d, expected errno=%d",
					 test_desc, TEST_ERRNO, exp_enos[0]);
			}
		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup(void)
 */
void setup()
{
	/* Capture unexpected signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be super/root for this test!");
		tst_exit();
	}

	TEST_PAUSE;

	/* Make a temp dir and cd to it */
	tst_tmpdir();
}

/*
 * cleanup()
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();

}