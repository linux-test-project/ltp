/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
 /*******************************************************************
 *
 *    TEST IDENTIFIER   : sched_setparam04
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : testing error conditions for sched_setparam(2)
 *
 *    TEST CASE TOTAL   : 4
 *
 *    AUTHOR            : Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 * DESCRIPTION
 * 	Verify that,
 *   1) sched_setparam(2) returns -1 and sets errno to ESRCH if the
 *	process with specified pid could not be found
 *   2) sched_setparam(2) returns -1 and sets errno to EINVAL if
 *	the parameter pid is an invalid value (-1)
 *   3) sched_setparam(2) returns -1 and sets errno to EINVAL if the
 *	parameter p is an invalid address
 *   4) sched_setparam(2) returns -1 sets errno to EINVAL if the
 *	value for p.sched_priority is other than 0 for scheduling
 *	policy, SCHED_OTHER
 *
 * ALGORITHM
 * Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if (system call failed (return=-1)) &
 *			   (errno set == expected errno)
 *              Issue sys call fails with expected return value and errno.
 *   Otherwise,
 *      Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *        Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_setparam04 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *		where,  -c n : Run n copies concurrently.
 *			-e   : Turn on errno logging.
 *			-h   : Show help screen
 *			-f   : Turn off functional testing
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-p   : Pause for SIGUSR1 before starting
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 *********************************************************************/

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sched.h>
#include <pwd.h>

#define LARGE_PID 999999

static void cleanup(void);
static void setup(void);

static struct sched_param param = { 0 };
static struct sched_param param1 = { 1 };

char *TCID = "sched_setparam04";
extern int Tst_count;

static int exp_enos[] = { EINVAL, ESRCH, 0 };	/* 0 terminated list of *
						 * expected errnos */

static struct test_case_t {
	char *desc;
	pid_t pid;
	struct sched_param *p;
	int exp_errno;
	char err_desc[10];
} test_cases[] = {
	{
	"test with non-existing pid", LARGE_PID, &param, ESRCH, "ESRCH"}, {
	"test invalid pid value", -1, &param, EINVAL, "EINVAL"}, {
	"test with invalid address for p", 0, NULL, EINVAL, "EINVAL"}, {
	"test with invalid p.sched_priority", 0, &param1, EINVAL,
		    "EINVAL"}
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{
	int lc, ind;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			/*
			 * call the system call with the TEST() macro
			 */
			TEST(sched_setparam(test_cases[ind].pid,
					    test_cases[ind].p));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == test_cases[ind].exp_errno)) {
				tst_resm(TPASS, "expected failure; Got %s",
					 test_cases[ind].err_desc);
			} else {
				tst_resm(TFAIL, "Call failed to produce "
					 "expected error;  Expected errno: %d "
					 "Got : %d, %s",
					 test_cases[ind].exp_errno,
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
			TEST_ERROR_LOG(TEST_ERRNO);
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

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;

}

/*
 * cleanup() -  performs all the ONE TIME cleanup for this test at completion
 *		or premature exit.
 */
void cleanup(void)
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
