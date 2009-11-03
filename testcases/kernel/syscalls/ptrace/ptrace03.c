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
 *    TEST IDENTIFIER	: ptrace03
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Tests for error conditions
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verifies that
 *	1) ptrace() returns -1 & sets errno to EPERM while tring to trace
 *	   process 1
 *         (This test case will be executed only if the kernel version
 *          is 2.6.25 or below)
 *	2) ptrace() returns -1 & sets errno to ESRCH if process with
 *	   specified pid does not exist
 *	3) ptrace() returns -1 & sets errno to EPERM if we are trying
 *	   to trace a process which is already been traced
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 setup signal handler for SIGUSR2 signal
 *	 fork a child
 *
 *	 CHILD:
 *	 	call ptrace() with proper arguments
 *	 	if ptrace() failed with expected return value & errno
 *			exit with errno
 *		else
 *			Give proper error message
 *			exit with errno
 *
 *	 PARENT:
 *		Wait for child to finish
 *		if child exits with expected errno
 *			Test Passed
 *	 	else
 *			Test failed
 *	
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  ptrace03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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
#include <sys/wait.h>
#include <pwd.h>

#include <config.h>
#include "ptrace.h"

#include "test.h"
#include "usctest.h"

#define INVALID_PID 999999

static void setup(void);
static void cleanup(void);

static int exp_enos[] = { EPERM, ESRCH, 0 };

char *TCID = "ptrace03";	/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */

struct test_case_t {
	enum __ptrace_request request;
	pid_t pid;
	int exp_errno;
} test_cases[] = {
	{
	PTRACE_ATTACH, 1, EPERM}, {
	PTRACE_ATTACH, INVALID_PID, ESRCH}, {
PTRACE_TRACEME, 0, EPERM},};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{

	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t child_pid;
	int status;

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

		for (i = 0; i < TST_TOTAL; ++i) {

			/* since Linux 2.6.26, it's allowed to trace init,
			   so just skip this test case */
			if (i == 0 && tst_kvercmp(2, 6, 25) > 0) {
				tst_resm(TCONF,
					 "this kernel allows to trace init");
				continue;
			}

			/* fork() */
			switch (child_pid = FORK_OR_VFORK()) {

			case -1:
				/* fork() failed */
				tst_resm(TFAIL, "fork() failed");
				continue;

			case 0:
				/* Child */

				/* setup for third test case */
				if (i == 2) {
					if ((ptrace(PTRACE_TRACEME, 0,
						    NULL, NULL)) == -1) {
						tst_resm(TWARN, "ptrace()"
							 " falied with errno, %d : %s",
							 errno,
							 strerror(errno));
						exit(0);
					}
				}

				TEST(ptrace(test_cases[i].request,
					    test_cases[i].pid, NULL, NULL));
				if ((TEST_RETURN == -1) && (TEST_ERRNO ==
							    test_cases[i].
							    exp_errno)) {
					exit(TEST_ERRNO);
				} else {
					tst_resm(TWARN|TTERRNO, "ptrace() returned %ld",
						 TEST_RETURN);
					exit(TEST_ERRNO);
				}

			default:
				/* Parent */
				if ((waitpid(child_pid, &status, 0)) < 0) {
					tst_resm(TFAIL, "waitpid() failed");
					continue;
				}
				if ((WIFEXITED(status)) &&
				    (WEXITSTATUS(status) ==
				     test_cases[i].exp_errno)) {
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

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	/* capture signals
	   tst_sig(FORK, DEF_HANDLER, cleanup); */

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
