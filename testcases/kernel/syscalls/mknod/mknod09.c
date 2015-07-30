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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#define MODE_RWX	S_IFMT	/* mode different from those expected */
#define TNODE		"tnode"	/*pathname */

char *TCID = "mknod09";
int TST_TOTAL = 1;

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;
	char *test_desc;	/* test specific error message */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		test_desc = "EINVAL";

		tst_count = 0;

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
				 EINVAL);
		} else {
			if (TEST_ERRNO == EINVAL) {
				tst_resm(TPASS, "mknod() fails with expected "
					 "error EINVAL errno:%d", TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "mknod() fails, %s, "
					 "errno=%d, expected errno=%d",
					 test_desc, TEST_ERRNO, EINVAL);
			}
		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup(void)
 */
void setup(void)
{
	tst_require_root();

	/* Capture unexpected signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Make a temp dir and cd to it */
	tst_tmpdir();
}

/*
 * cleanup()
 */
void cleanup(void)
{

	tst_rmdir();

}
