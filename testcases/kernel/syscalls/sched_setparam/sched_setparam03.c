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
 *    TEST IDENTIFIER	: sched_setparam03
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Checks functionality for sched_setparam(2) for pid!=0
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
 *	This test forks a child & changes its parent's scheduling priority
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Change scheduling policy to SCHED_FIFO
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 Fork a child
 *
 *	 CHILD:
 *	  Changes scheduling priority for parent
 *
 *	 PARENT:
 *	  If scheduling priority is set properly,
 *		TEST passed
 *	  else
 *		TEST failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_setparam03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

#include <err.h>
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

#define NEW_PRIORITY 5

static void setup();
static void cleanup();
static int verify_priority();

char *TCID = "sched_setparam03";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

static struct sched_param param = { NEW_PRIORITY };

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int status;
	pid_t child_pid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		switch (child_pid = FORK_OR_VFORK()) {

		case -1:
			/* fork() failed */
			tst_resm(TFAIL, "fork() failed");
			continue;

		case 0:
			/* Child */

			/*
			 * Call sched_setparam(2) with pid = getppid() so that
			 * it will set the scheduling parameters for parent
			 * process
			 */
			TEST(sched_setparam(getppid(), &param));

			if (TEST_RETURN == -1) {
				err(0, "sched_setparam returned %ld",
				    TEST_RETURN);
			}
			exit(1);

		default:
			/* Parent */
			if ((waitpid(child_pid, &status, 0)) < 0) {
				tst_resm(TFAIL, "wait() failed");
				continue;
			}

			/*
			 * Verify that parent's scheduling priority has
			 * changed.
			 */
			if ((WIFEXITED(status)) && (WEXITSTATUS(status)) &&
			    (verify_priority())) {
				tst_resm(TPASS, "Test Passed");
			} else {
				tst_resm(TFAIL, "Test Failed");
			}
		}
	}

	cleanup();
	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	struct sched_param p = { 1 };

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Change scheduling policy to SCHED_FIFO */
	if ((sched_setscheduler(0, SCHED_FIFO, &p)) == -1) {
		tst_brkm(TBROK, cleanup, "sched_setscheduler() failed");
	}

}

/*
 *cleanup() -   performs all ONE TIME cleanup for this test at
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

/*
 * verify_priority() -  This function checks whether the priority is
 *			set correctly
 */
int verify_priority()
{
	struct sched_param p;

	if ((sched_getparam(0, &p)) == 0) {
		if (p.sched_priority == NEW_PRIORITY) {
			return 1;
		} else {
			tst_resm(TWARN, "sched_getparam() returned priority"
				 " value as %d", p.sched_priority);
			return 0;
		}
	}

	tst_resm(TWARN, "sched_getparam() failed");
	return 0;
}