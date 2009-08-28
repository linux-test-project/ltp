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
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EINVAL, 0 };

gid_t groups_list[NGROUPS];	/* buffer to hold user group list */

int no_setup();
void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	size_t gsize;
	gid_t list;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	-1, 1, "Size is < no. suppl. gids", EINVAL, no_setup}, {
	0, 0, NULL, 0, no_setup}
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int gidsetsize;		/* total no. of groups */
	int ind;		/* counter to test different test conditions */
	char *test_desc;	/* test specific error message */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -c option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			gidsetsize = Test_cases[ind].gsize;
			test_desc = Test_cases[ind].desc;

			/*
			 * Call getgroups() to test different test conditions
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(getgroups(gidsetsize, groups_list));

			/* check return code of getgroups(2) */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == Test_cases[ind].exp_errno) {
					tst_resm(TPASS, "getgroups() fails, %s,"
						 " errno=%d", test_desc,
						 TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "getgroups() fails, %s,"
						 " errno=%d, expected errno=%d",
						 test_desc, TEST_ERRNO,
						 Test_cases[ind].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "getgroups() returned %ld, "
					 "expected -1, errno=%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}		/* End of TEST CASE LOOPING. */

	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

/*
 * no_setup() - Some test conditions for mknod(2) do not any setup.
 *              Hence, this function just returns 0.
 *  		This function simply returns 0.
 */
int no_setup()
{
	return 0;
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

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
