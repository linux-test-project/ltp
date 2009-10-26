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
 * Test Name: getpriority02
 *
 * Test Description:
 *  Verify that,
 *   1) getpriority() sets errno to ESRCH  if no process was located
 *	was located for 'which' and 'who' arguments.
 *   2) getpriority() sets errno to EINVAL if 'which' argument was
 *      not one of PRIO_PROCESS, PRIO_PGRP, or PRIO_USER.
 *
 * Expected Result:
 *  getpriority() should fail with return value -1 and set expected errno.
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
 *  getpriority02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
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
 *  None.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "test.h"
#include "usctest.h"

#define INVAL_PID	-1
#define INVAL_FLAG      -1

char *TCID = "getpriority02";	/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EINVAL, ESRCH, 0 };

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	int pro_which;
	uid_t pro_uid;
	char *desc;
	int exp_errno;
} Test_cases[] = {
	{
	INVAL_FLAG, 0, "Invalid 'which' value specified", EINVAL}, {
	PRIO_PROCESS, INVAL_PID, "Invalid 'who' value specified", ESRCH}, {
	PRIO_PGRP, INVAL_PID, "Invalid 'who' value specified", ESRCH}, {
	PRIO_USER, INVAL_PID, "Invalid 'who' value specified", ESRCH}, {
	0, 0, NULL, 0}
};

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int ind;		/* counter variable for test case looping */
	char *test_desc;	/* test specific error message */
	int which;		/* process priority category */
	uid_t who;		/* process uid of the test process */

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

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			which = Test_cases[ind].pro_which;
			who = Test_cases[ind].pro_uid;
			test_desc = Test_cases[ind].desc;

			if (who == 0) {
				/* Get the actual uid of the process */
				who = getuid();
			}

			/*
			 * Invoke getpriority with the specified
			 * 'which' and 'who' arguments and verify
			 * that it fails with expected errno.
			 */
			TEST(getpriority(which, who));

			/* check return code from getpriority(2) */
			if (TEST_RETURN < 0) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == Test_cases[ind].exp_errno) {
					tst_resm(TPASS, "getpriority(2) fails, "
						 "%s, errno:%d",
						 test_desc, TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "getpriority() fails, "
						 "%s, errno:%d, expected errno:"
						 "%d", test_desc, TEST_ERRNO,
						 Test_cases[ind].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "getpriority() returned %ld, "
					 "expected -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}		/* End of TEST CASE LOOPING. */

	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
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

	/* exit with return code appropriate for results */
	tst_exit();
}
