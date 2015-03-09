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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**************************************************************************
 *
 *    TEST IDENTIFIER	: timer_settime02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for timer_settime(2)
 *
 *    TEST CASE TOTAL	: 4
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *     This is a Phase I test for the timer_settime(2) system call.
 *     It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 setup individual test
 *	 Execute system call
 *	 Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	 Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * timer_settime02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
 * where:
 * 	-c n : Run n copies simultaneously.
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 * None
 *****************************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "test.h"
#include "common_timers.h"

void setup(void);
static int setup_test(int option);

char *TCID = "timer_settime02";	/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */

static struct itimerspec new_set, old_set, *old_temp;
static kernel_timer_t timer;
static int flag;

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/* Set up individual test */
			if (setup_test(i) == 0) {
				TEST(ltp_syscall(__NR_timer_settime, timer,
					flag, &new_set, old_temp));
				tst_resm((TEST_RETURN == 0 ?
					  TPASS :
					  TFAIL | TTERRNO),
					 "%s",
					 (TEST_RETURN ==
					  0 ? "passed" : "failed")
				    );
			}

		}
	}

	cleanup();
	tst_exit();
}

/* This function does set up for individual tests */
static int setup_test(int option)
{
	struct timespec timenow;	/* Used to obtain current time */
	int rc = 0;

	switch (option) {
	case 0:
		/* This is general initialization.
		 * make old_setting NULL
		 * make flags equal to zero
		 * use one-shot timer
		 */
		old_temp = NULL;
		new_set.it_interval.tv_sec = 0;
		new_set.it_interval.tv_nsec = 0;
		new_set.it_value.tv_sec = 5;
		new_set.it_value.tv_nsec = 0;
		flag = 0;
		break;
	case 1:
		/* get the old_setting in old_set
		 * This test case also takes care
		 * of situation where the timerid is
		 * already armed
		 */
		old_temp = &old_set;
		break;
	case 2:
		/* Use the periodic timer */
		new_set.it_interval.tv_sec = 5;
		new_set.it_value.tv_sec = 0;
		break;
	case 3:
		/* Use TIMER_ABSTIME flag for setting
		 * absolute time for timer
		 */
		flag = TIMER_ABSTIME;
		/*
		 * Let's not use the linux_syscall_number syscall(2)
		 * wrapper here because our primary test focus is
		 * timer_create, not clock_gettime. That's covered in
		 * those respective tests.
		 */
		if (clock_gettime(CLOCK_REALTIME, &timenow) < 0) {
			tst_resm(TWARN | TERRNO,
				 "clock_gettime failed; skipping the test");
			rc = -1;
		} else {
			new_set.it_value.tv_sec = timenow.tv_sec + 25;
		}
		break;
	}
	return rc;
}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	if (ltp_syscall(__NR_timer_create, CLOCK_REALTIME, NULL, &timer) < 0)
		tst_brkm(TBROK, NULL, "timer_create failed");
}

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */
void cleanup(void)
{
}
