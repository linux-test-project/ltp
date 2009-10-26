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
 *    TEST IDENTIFIER	: getrusage02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Tests for error conditions
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that
 *	1) getrusage() fails with errno EINVAL when an invalid value
 *	   is given for who
 *	2) getrusage() fails with errno EFAULT when an invalid address
 *	   is given for usage
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  if call failed with expected errno,
 *		Test Passed
 *	  else
 *		Test Failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  getrusage02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f]
 * 			     [-p]
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
#include <sys/resource.h>
#include "test.h"
#include "usctest.h"

#ifndef RUSAGE_BOTH		/* Removed from user space on RHEL4 */
#define RUSAGE_BOTH (-2)	/* still works on SuSE      */
#endif /* so this is a work around */

static void setup();
static void cleanup();
static int exp_enos[] = { EINVAL, EFAULT, 0 };

char *TCID = "getrusage02";	/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */

static struct rusage usage;

struct test_cases_t {
	int who;
	struct rusage *usage;
	int exp_errno;
} test_cases[] = {
	{
	RUSAGE_BOTH, &usage, EINVAL},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	{
	RUSAGE_SELF, (struct rusage *)-1, EFAULT}
#endif
};

int TST_TOTAL = sizeof(test_cases) / sizeof(*test_cases);

int main(int ac, char **av)
{

	int lc, ind;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			/*
			 * Call getrusage(2)
			 */
			TEST(getrusage(test_cases[ind].who,
				       test_cases[ind].usage));

			if ((TEST_RETURN == -1) && (TEST_ERRNO ==
						    test_cases[ind].
						    exp_errno)) {
				tst_resm(TPASS, "TEST Passed");
			} else {
				tst_resm(TFAIL, "test Failed,"
					 "getrusage() returned %ld"
					 " errno = %d : %s", TEST_RETURN,
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
			TEST_ERROR_LOG(TEST_ERRNO);
		}
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

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

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
