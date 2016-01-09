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
 *    TEST IDENTIFIER	: timer_create02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for timer_create(2)
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *     This is a Phase I test for the timer_create(2) system call.
 *     It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 Execute system call with different notification types for
 *	 clock ID CLOCK_REALTIME
 *	 Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	 Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * timer_create02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
void setup_test(int option);

char *TCID = "timer_create02";	/* Test program identifier.    */
int TST_TOTAL = 3;		/* Total number of test cases. */
static struct sigevent evp, *evp_ptr;

int main(int ac, char **av)
{
	int lc, i, j;
	kernel_timer_t created_timer_id;	/* holds the returned timer_id */
	char *message[3] = {
		"SIGEV_SIGNAL",
		"NULL",
		"SIGEV_NONE"
	};
	const char *mrstr = "MONOTONIC_RAW";

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			setup_test(i);

			for (j = 0; j < CLOCKS_DEFINED; ++j) {

				if (strstr(get_clock_str(clock_list[j]),
					   "CPUTIME_ID")) {
					/* (PROCESS_CPUTIME_ID &
					 *  THREAD_CPUTIME_ID)
					 * is not supported on kernel versions
					 * lower than 2.6.12
					 */
					if ((tst_kvercmp(2, 6, 12)) < 0) {
						continue;
					}
				}
				if (strstr(get_clock_str(clock_list[j]), mrstr))
					continue;

				TEST(ltp_syscall(__NR_timer_create,
					clock_list[j], evp_ptr,
					&created_timer_id));

				tst_resm((TEST_RETURN == 0 ?
					  TPASS :
					  TFAIL | TTERRNO),
					 "%s %s with notification type = %s",
					 get_clock_str(clock_list[j]),
					 (TEST_RETURN == 0 ?
					  "passed" : "failed"), message[i]);
			}
		}
	}

	cleanup();
	tst_exit();
}

/* setup_test() - sets up individual test */
void setup_test(int option)
{
	switch (option) {
	case 0:
		evp.sigev_value = (union sigval) 0;
		evp.sigev_signo = SIGALRM;
		evp.sigev_notify = SIGEV_SIGNAL;
		evp_ptr = &evp;
		break;
	case 1:
		evp_ptr = NULL;
		break;
	case 2:
		evp.sigev_value = (union sigval) 0;
		evp.sigev_signo = SIGALRM;	/* any will do */
		evp.sigev_notify = SIGEV_NONE;
		evp_ptr = &evp;
		break;
	}
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
