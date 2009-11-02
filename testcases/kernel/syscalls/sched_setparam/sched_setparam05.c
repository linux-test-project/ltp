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
 *    TEST IDENTIFIER	: sched_setparam05
 *
 *    EXECUTED BY	: root/superuser
 *
 *    TEST TITLE	: verify that sched_setparam() fails if the user does
 *			  not have proper privilages
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that sched_setparam() fails if the user does
 *	not have proper privilages
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 Fork a child
 *
 *	 CHILD:
 *	  Changes euid to "nobody" user.
 *	  Try to Change scheduling priority for parent
 *	  If call failed with errno = EPERM,
 *		Test passed
 *	  else
 *		Test failed
 *
 *	 PARENT:
 *		wait for child to finish
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_setparam05 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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
#include <pwd.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

static void setup();
static void cleanup();

char *TCID = "sched_setparam05";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

static struct sched_param param = { 0 };
static int exp_enos[] = { EPERM, 0 };
static char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int status;
	pid_t child_pid;

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

		switch (child_pid = FORK_OR_VFORK()) {

		case -1:
			/* fork() failed */
			tst_resm(TFAIL, "fork() failed");
			continue;

		case 0:
			/* Child */

			/* Switch to nobody user */
			if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
				tst_brkm(TBROK, tst_exit, "\"nobody\" user"
					 "not present");
			}
			if (seteuid(ltpuser->pw_uid) == -1) {
				tst_resm(TWARN, "seteuid failed to "
					 "to set the effective uid to %d",
					 ltpuser->pw_uid);
				exit(1);
			}

			/*
			 * Call sched_setparam(2) with pid = getppid()
			 */
			TEST(sched_setparam(getppid(), &param));

			if ((TEST_RETURN == -1) && (TEST_ERRNO == EPERM)) {
				TEST_ERROR_LOG(TEST_ERRNO);
				exit(0);
			}

			tst_resm(TWARN|TTERRNO, "Test failed, sched_setparam()"
				 " returned : %ld",
				 TEST_RETURN);
			TEST_ERROR_LOG(TEST_ERRNO);
			exit(1);

		default:
			/* Parent */
			if ((waitpid(child_pid, &status, 0)) < 0) {
				tst_resm(TFAIL, "wait() failed");
				continue;
			}
			if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 0)) {
				tst_resm(TPASS, "Test passed, Got EPERM");
			} else {
				tst_resm(TFAIL, "Test Failed");
			}
		}
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	/* Check whether we are root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

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
