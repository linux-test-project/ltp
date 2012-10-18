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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: sched_rr_get_interval03
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Tests for error conditions
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that
 *	1) sched_rr_get_interval() fails with errno set to EINVAL for an
 *	   invalid pid
 *	2) sched_rr_get_interval() fails with errno set to ESRCH if the
 *	   process with specified pid does not exists
 *	2) sched_rr_get_interval() fails with errno set to EFAULT if the
 *	   address specified as &tp is invalid
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Set expected errors for logging.
 *	  Pause for SIGUSR1 if option specified.
 *	  Change scheduling policy to SCHED_RR
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  if call fails with expected errno,
 *		Test passed.
 *	  Otherwise
 *		Test failed
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * sched_rr_get_interval03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <errno.h>
#include <sched.h>
#include "test.h"
#include "usctest.h"

#define PID_DONT_EXISTS 999999

static void setup();
static void cleanup();

char *TCID = "sched_rr_get_interval03";	/* Test program identifier.    */
struct timespec tp;
static int exp_enos[] = { EINVAL, ESRCH, EFAULT, 0 };

struct test_cases_t {
	pid_t pid;
	struct timespec *tp;
	int exp_errno;
} test_cases[] = {
	{
	-1, &tp, EINVAL}, {
	PID_DONT_EXISTS, &tp, ESRCH},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	{
	0, (struct timespec *)-1, EFAULT}
#endif
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{

	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			/*
			 * Call sched_rr_get_interval(2)
			 */
			TEST(sched_rr_get_interval(test_cases[i].pid,
						   test_cases[i].tp));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == test_cases[i].exp_errno)) {
				tst_resm(TPASS, "Test Passed");
			} else {
				tst_resm(TFAIL|TTERRNO, "Test Failed,"
					 " sched_rr_get_interval() returned %ld", TEST_RETURN);
			}
			TEST_ERROR_LOG(TEST_ERRNO);
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	/*
	 * Initialize scheduling parameter structure to use with
	 * sched_setscheduler()
	 */
	struct sched_param p = { 1 };

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	/* Change scheduling policy to SCHED_RR */
	if ((sched_setscheduler(0, SCHED_RR, &p)) == -1) {
		tst_brkm(TBROK, cleanup, "sched_setscheduler() failed");
	}
}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
