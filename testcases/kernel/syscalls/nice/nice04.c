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
 * Test Name: nice04
 *
 * Test Description:
 *  Verify that, nice(2) fails when, a non-root user attempts to increase
 *  the priority of a process by specifying a negative increment value.
 *
 * Expected Result:
 *  nice() returns -1 and sets errno to EPERM.
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
 *		PASS
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		FAIL
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  nice04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *  This test should be executed by 'non-super-user' only.
 */
#include <pwd.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

char *TCID = "nice04";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int exp_enos[] = { EPERM, 0 };

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	int nice_val;
	char *desc;
	int exp_errno;
} Test_cases[] = {
	{
	-5, "Non-root cannot specify higher priority", EPERM}
};

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;			/* counter variable for test case looping */
	int incr_val;		/* nice value for the process */
	char *test_desc;	/* test specific error message */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			incr_val = Test_cases[i].nice_val;
			test_desc = Test_cases[i].desc;

			/*
			 * Call nice(2) with different 'incr' parameter
			 * values and verify that it fails as expected.
			 */
			TEST(nice(incr_val));

			/* check return code from nice(2) */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TPASS, "nice(2) returned %ld for %s",
					 TEST_RETURN, test_desc);
			} else {
				tst_resm(TFAIL|TTERRNO,
					 "nice() returned %ld for %s",
					 TEST_RETURN, test_desc);
			}
		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *  Make sure the test process uid is non-root only.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

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
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}