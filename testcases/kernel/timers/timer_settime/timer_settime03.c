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
 *    TEST IDENTIFIER	: timer_settime03
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Test checking for basic error conditions for
 *    			  timer_settime(2)
 *
 *    TEST CASE TOTAL	: 6
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *    	This test case check whether timer_settime(2) returns appropriate error
 *    	value for invalid parameter
 *
 * 	Setup:
 *	 Setup signal handling.
 *	 Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 setup the individual test.
 *	 Execute system call with invalid parameters.
 *	 Check return code, if system call fails with errno == expected errno
 * 	 	Issue syscall passed with expected errno
 *	 Otherwise, Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 	 Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * timer_settime03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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

#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "test.h"
#include "common_timers.h"

void setup(void);
void setup_test(int option);

int testcase[] = {
	EINVAL,			/* New setting null */
	EINVAL,			/* tv_nsec < 0 */
	EINVAL,			/* nsec > NSEC/SEC */
	EINVAL,			/* Invalid timerid */
	EFAULT,			/* bad newsetting * */
	EFAULT			/* bad oldsetting * */
};

char *TCID = "timer_settime03";
int TST_TOTAL = ARRAY_SIZE(testcase);

static struct itimerspec new_set, old_set, *old_temp, *new_temp;
static kernel_timer_t timer, tim;

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/* Set up individual tests */
			setup_test(i);
			TEST(ltp_syscall(__NR_timer_settime, tim, 0, new_temp,
				     old_temp));

			/* check return code */
			if (TEST_RETURN == -1 && TEST_ERRNO == testcase[i]) {
				tst_resm(TPASS | TTERRNO, "failed as expected");
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "didn't fail as expected [expected "
					 "errno = %d (%s)]",
					 testcase[i], strerror(testcase[i]));
			}	/* end of else */

		}

	}

	cleanup();
	tst_exit();
}

/* This function sets up individual tests */
void setup_test(int option)
{
	switch (option) {
	case 0:
		/* Pass NULL structure as new setting */
		new_temp = NULL;
		tim = timer;
		old_temp = &old_set;
		break;
	case 1:
		/* Make tv_nsec value less than 0 */
		new_set.it_value.tv_nsec = -1;
		new_set.it_value.tv_sec = 5;
		new_set.it_interval.tv_sec = 0;
		new_set.it_interval.tv_nsec = 0;
		new_temp = &new_set;
		break;
	case 2:
		/* Make tv_nsec more than NSEC_PER_SEC */
		new_set.it_value.tv_nsec = NSEC_PER_SEC + 1;
		break;
	case 3:
		/* make timer_id invalid */
		tim = (kernel_timer_t) - 1;
		new_set.it_value.tv_nsec = 0;
		break;
	case 4:
		/* make new_setting a bad pointer */
		tim = timer;
		new_temp = (struct itimerspec *)-1;
		break;
	case 5:
		/* make old_setting a bad pointer */
		new_temp = &new_set;
		old_temp = (struct itimerspec *)-1;
		break;
	}
}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	if (ltp_syscall(__NR_timer_create, CLOCK_REALTIME, NULL, &timer) < 0) {
		tst_brkm(TBROK, NULL, "Timer create failed. Cannot"
			 " setup test");
	}

	TEST_PAUSE;
}

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */
void cleanup(void)
{
}
