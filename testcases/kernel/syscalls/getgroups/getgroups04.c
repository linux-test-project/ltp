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
 * Test Name: getgroups04
 *
 * Test Description:
 *  Verify that,
 *   getgroups() fails with -1 and sets errno to EINVAL if the size
 *   argument value is -ve.
 *
 * Expected Result:
 *  getgroups() should fail with return value -1 and set expected errno.
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
 *  getgroups04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be executed by non-super-user only.
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "test.h"
#include "usctest.h"

char *TCID = "getgroups04";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test conditions */
int exp_enos[] = { EINVAL, 0 };

gid_t groups_list[NGROUPS];	/* buffer to hold user group list */

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	size_t gsize;
	gid_t list;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} test_cases[] = {
	{ -1, 1, "Size is < no. suppl. gids", EINVAL },
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int gidsetsize;		/* total no. of groups */
	int i;		/* counter to test different test conditions */
	char *test_desc;	/* test specific error message */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			gidsetsize = test_cases[i].gsize;
			test_desc = test_cases[i].desc;

			TEST(getgroups(gidsetsize, groups_list));

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == test_cases[i].exp_errno)
					tst_resm(TPASS|TTERRNO,
					    "getgroups failed as expected");
				else
					tst_resm(TFAIL|TTERRNO,
					    "getgroups failed unexpectedly; "
					    "expected: %d - %s",
					    test_cases[i].exp_errno,
					    strerror(test_cases[i].exp_errno));
			} else
				tst_resm(TFAIL,
				    "getgroups succeeded unexpectedly");
		}

	}

	cleanup();

	tst_exit();
}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

}