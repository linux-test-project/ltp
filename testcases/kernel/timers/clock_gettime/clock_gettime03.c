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
 *    TEST IDENTIFIER	: clock_gettime03
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Test checking for basic error conditions for
 *    			  clock_gettime(2)
 *
 *    TEST CASE TOTAL	: 7
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *    	This test case check whether clock_gettime(2) returns appropriate error
 *    	value for invalid parameter
 *
 * 	Setup:
 *	 Setup signal handling.
 *	 Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 If it is the first test case
 *	 	make temp a bad pointer
 *	 Otherwise pass defined struct timespec variable to temp
 *	 Execute system call with invalid parameter
 *	 Check return code, if system call fails with errno == expected errno
 * 	 	Issue syscall passed with expected errno
 *	 Otherwise, Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 	 Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * clock_gettime03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
#include <time.h>
#include <signal.h>

#include "test.h"
#include "common_timers.h"

void setup(void);

int testcase[6] = {
	EFAULT,			/* Bad timespec   */
	EFAULT,			/* Bad timespec   */
	EINVAL,			/* MAX_CLOCKS     */
	EINVAL			/* MAX_CLOCKS + 1 */
};

char *TCID = "clock_gettime03";	/* Test program identifier.    */
int TST_TOTAL = ARRAY_SIZE(testcase);

int main(int ac, char **av)
{
	int i, lc;
	struct timespec spec, *temp;

	clockid_t clocks[] = {
		CLOCK_REALTIME,
		CLOCK_MONOTONIC,
		MAX_CLOCKS,
		MAX_CLOCKS + 1,
		CLOCK_PROCESS_CPUTIME_ID,
		CLOCK_THREAD_CPUTIME_ID
	};

	tst_parse_opts(ac, av, NULL, NULL);

	/*
	 * PROCESS_CPUTIME_ID & THREAD_CPUTIME_ID are not supported on
	 * kernel versions lower than 2.6.12
	 */
	if ((tst_kvercmp(2, 6, 12)) < 0) {
		testcase[4] = EINVAL;
		testcase[5] = EINVAL;
	} else {
		testcase[4] = EFAULT;
		testcase[5] = EFAULT;
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			temp = &spec;

			if (i == 0) {
				temp = (struct timespec *)-1;
			} else if (i == 1) {
				temp = NULL;
			} else if ((i >= 4) && (tst_kvercmp(2, 6, 12) >= 0)) {
				temp = NULL;
			}

			TEST(ltp_syscall(__NR_clock_gettime, clocks[i], temp));

			/* check return code */
			if (TEST_RETURN == -1 && TEST_ERRNO == testcase[i]) {
				tst_resm(TPASS | TTERRNO,
					 "got expected failure");
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "failed to produce expected error "
					 "[expected errno = %d (%s), "
					 "TEST_RETURN = %ld]",
					 testcase[i], strerror(testcase[i]),
					 TEST_RETURN);
			}	/* end of else */

		}		/*End of TEST CASE LOOPING */

	}			/*End for TEST_LOOPING */

	cleanup();
	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */
void cleanup(void)
{
}
