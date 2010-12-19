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
 *    TEST IDENTIFIER	: prctl02
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
 *	1) prctl() fails with errno, EINVAL when an invalid value is given for
 *	   option
 *	2) prctl() fails with errno, EINVAL when option is PR_SET_PDEATHSIG
 *	   & arg2 is not zero or a valid signal number
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 fork a child
 *
 *	 CHILD:
 *		call prctl() with proper arguments
 *		If call fails with expected errno,
 *			exit with 0
 *		else
 *			exit with 1
 *	 PARENT:
 *		wait() for child.
 *		If child exits with exit value 0,
 *			Test passed
 *		else
 *			Test Failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  prctl02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

#define OPTION_INVALID 999
#define INVALID_ARG 999

static void setup(void);
static void cleanup(void);

char *TCID = "prctl02";		/* Test program identifier.    */
static int exp_enos[] = { EINVAL, EINVAL, 0 };

struct test_cases_t {
	int option;
	unsigned long arg2;
	int exp_errno;
} test_cases[] = {
	{
	OPTION_INVALID, 0, EINVAL}, {
	PR_SET_PDEATHSIG, INVALID_ARG, EINVAL}
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{

	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t child_pid;
	int status;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			switch (child_pid = FORK_OR_VFORK()) {

			case -1:
				/* fork() failed */
				tst_resm(TFAIL, "fork() failed");
				continue;

			case 0:
				/* Child */

				TEST(prctl(test_cases[i].option,
					   test_cases[i].arg2));
				if ((TEST_RETURN == -1) && (TEST_ERRNO ==
							    test_cases[i].
							    exp_errno)) {
					exit(TEST_ERRNO);
				} else {
					tst_resm(TWARN|TTERRNO, "prctl() returned %ld",
						 TEST_RETURN);
					exit(TEST_ERRNO);
				}

			default:
				/* Parent */
				if ((waitpid(child_pid, &status, 0)) < 0) {
					tst_resm(TFAIL, "waitpid() failed");
					continue;
				}

				if ((WIFEXITED(status)) && (WEXITSTATUS(status)
							    == test_cases[i].
							    exp_errno)) {
					tst_resm(TPASS, "Test Passed");
				} else {
					tst_resm(TFAIL, "Test Failed");
				}
				TEST_ERROR_LOG(WEXITSTATUS(status));

			}
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

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