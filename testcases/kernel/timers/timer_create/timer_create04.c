/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
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
/**************************************************************************
 *
 *    TEST IDENTIFIER	: timer_create04
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Test checking for basic error conditions for
 *    			  timer_create(2)
 *
 *    TEST CASE TOTAL	: 8
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *    	This test case check whether timer_create(2) returns appropriate error
 *    	value for invalid parameter
 *
 * 	Setup:
 *	 Setup signal handling.
 *	 Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 For case 7 set event parameter as bad pointer
 *	 for case 8 set returned timer ID parameter as bad pointer
 *	 Execute system call with invalid parameter
 *	 Check return code, if system call fails with errno == expected errno
 * 	 	Issue syscall passed with expected errno
 *	 Otherwise, Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 	 Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * timer_create04 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
 * where:
 * 	-c n : run n copies simultaneously
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 * RESTRICTIONS:
 * None
 *****************************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "test.h"
#include "usctest.h"
#include "common_timers.h"

void setup(void);

char *TCID = "timer_create04"; 	/* Test program identifier.    */
int TST_TOTAL;			/* Total number of test cases. */

static int exp_enos[] = {EINVAL, EFAULT, 0};

int testcase[6] = {
	EINVAL,	/* MAX_CLOCKS     */
	EINVAL,	/* MAX_CLOCKS + 1 */
	EFAULT,	/* bad sigevent   */
	EFAULT	/* bad timer_id   */
};

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */
void
cleanup(void)
{
	/*
	* print timing stats if that option was specified.
	* print errno log if that option was specified.
	*/
	TEST_CLEANUP;

}

int
main(int ac, char **av)
{
	int lc, i;			/* loop counter */
	char *msg;			/* message returned from parse_opts */
	kernel_timer_t timer_id, *temp_id;	/* stores the returned timer_id */
	struct sigevent *temp_ev;	/* used for bad address test case */

	clockid_t clocks[6] = {
		MAX_CLOCKS,
		MAX_CLOCKS + 1,
		CLOCK_REALTIME,
		CLOCK_MONOTONIC,
		CLOCK_PROCESS_CPUTIME_ID,
		CLOCK_THREAD_CPUTIME_ID
	};

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	TST_TOTAL = sizeof(testcase) / sizeof(testcase[0]);

	/* PROCESS_CPUTIME_ID & THREAD_CPUTIME_ID are not supported on
	 * kernel versions lower than 2.6.12
	 */
	if (tst_kvercmp(2, 6, 12) < 0) {
		testcase[4] = EINVAL;
		testcase[5] = EINVAL;
	} else {
		testcase[4] = EFAULT;
		testcase[5] = EFAULT;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			temp_ev = (struct sigevent *) NULL;
			temp_id = &timer_id;

			switch (i) {
			case 2: /* make the timer_id bad address */
				temp_id = (kernel_timer_t *) -1;
				break;
			case 3:
				/* make the event bad address */
				temp_ev = (struct sigevent *) -1;
				break;
			case 4:
				/* Produce an invalid timer_id address. */
				if (tst_kvercmp(2, 6, 12) >= 0)
					temp_id = (kernel_timer_t *) -1;
				break;
			case 5:
				/* Produce an invalid event address. */
				if (tst_kvercmp(2, 6, 12) >= 0)
					temp_ev = (struct sigevent *) -1;
			}

			TEST(syscall(__NR_timer_create, clocks[i], temp_ev,
					temp_id));

			/* check return code */
			if (TEST_RETURN == -1 && TEST_ERRNO == testcase[i]) {
				tst_resm(TPASS | TTERRNO, "failed as expected");
			} else {
				tst_resm(TFAIL | TTERRNO,
					"didn't fail as expected [expected "
					"errno = %d (%s)]",
					testcase[i],
					strerror(testcase[i]));
			} /* end of else */

		}

	}

	cleanup();
	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test */
void
setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}