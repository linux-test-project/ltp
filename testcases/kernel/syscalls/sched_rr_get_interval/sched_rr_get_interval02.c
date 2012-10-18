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
 *    TEST IDENTIFIER	: sched_rr_get_interval02
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Functionality test
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that for a process with scheduling policy SCHED_FIFO,
 *	sched_rr_get_interval() writes zero into timespec structure
 *	for tv_sec & tv_nsec.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Change scheduling policy to SCHED_FIFO
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check (return code == 0) & (got 0 for tv_sec & tv_nsec )
 *		Test passed.
 *	  Otherwise
 *		Test failed
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * sched_rr_get_interval02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

static void setup();
static void cleanup();

char *TCID = "sched_rr_get_interval02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

struct timespec tp;

int main(int ac, char **av)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		tp.tv_sec = 99;
		tp.tv_nsec = 99;
		/*
		 * Call sched_rr_get_interval(2) with pid=0 sothat it will
		 * write into the timespec structure pointed to by tp the
		 * round robin time quantum for the current process.
		 */
		TEST(sched_rr_get_interval(0, &tp));

		if ((TEST_RETURN == 0) && (tp.tv_sec == 0) && (tp.tv_nsec == 0)) {
			tst_resm(TPASS, "Test passed");
		} else {
			tst_resm(TFAIL, "Test Failed, sched_rr_get_interval()"
				 "returned %ld, errno = %d : %s, tp.tv_sec = %d,"
				 " tp.tv_nsec = %ld", TEST_RETURN, TEST_ERRNO,
				 strerror(TEST_ERRNO), (int)tp.tv_sec,
				 tp.tv_nsec);
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

	TEST_PAUSE;

	/* Change scheduling policy to SCHED_FIFO */
	if ((sched_setscheduler(0, SCHED_FIFO, &p)) == -1) {
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
