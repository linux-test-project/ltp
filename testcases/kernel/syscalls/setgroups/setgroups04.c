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
 * Test Name: setgroups04
 *
 * Test Description:
 *  Verify that, setgroups() fails with -1 and sets errno to EFAULT if the list has an invalid address.
 *
 * Expected Result:
 *  setgroups() should fail with return value -1 and set expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	if errno set == expected errno
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  setgroups04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
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
 *  none.
 *
 */
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include "test.h"

/*
 * Don't forget to remove USE_LEGACY_COMPAT_16_H from Makefile after
 * rewriting this test to the new API.
 */
#include "compat_16.h"

TCID_DEFINE(setgroups04);
int TST_TOTAL = 1;

GID_T groups_list[NGROUPS];

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;
	int gidsetsize;		/* total no. of groups */
	char *test_desc;	/* test specific error message */

	tst_parse_opts(ac, av, NULL, NULL);

	/* Perform setup for test */
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		gidsetsize = NGROUPS;
		test_desc = "EFAULT";

		/*
		 * Call setgroups() to test condition
		 * verify that it fails with -1 return value and
		 * sets appropriate errno.
		 */
		TEST(SETGROUPS(cleanup, gidsetsize, sbrk(0)));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "setgroups() returned %ld, "
				 "expected -1, errno=%d", TEST_RETURN,
				 EFAULT);
		} else {

			if (TEST_ERRNO == EFAULT) {
				tst_resm(TPASS,
					 "setgroups() fails with expected "
					 "error EFAULT errno:%d", TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "setgroups() fails, %s, "
					 "errno=%d, expected errno=%d",
					 test_desc, TEST_ERRNO, EFAULT);
			}
		}

	}

	cleanup();
	tst_exit();

}

#else

int main(void)
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */

/*
 * setup()
 */
void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 * cleanup()
 */
void cleanup(void)
{

}
