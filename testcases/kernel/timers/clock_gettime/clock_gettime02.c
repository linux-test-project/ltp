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
 *    TEST IDENTIFIER	: clock_gettime02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for clock_gettime(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *     This is a Phase I test for the clock_gettime(2) system call.
 *     It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 Execute system call for each defined clock value
 *	 Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	 Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * clock_gettime02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
static clockid_t clocks[2] = { CLOCK_REALTIME, CLOCK_MONOTONIC };

char *TCID = "clock_gettime02";
int TST_TOTAL = ARRAY_SIZE(clocks);

int main(int ac, char **av)
{
	int lc, i;
	struct timespec spec;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(ltp_syscall(__NR_clock_gettime, clocks[i], &spec));
			tst_resm((TEST_RETURN < 0 ? TFAIL | TTERRNO : TPASS),
				 "%s",
				 (TEST_RETURN == 0 ? "passed" : "failed"));
		}
	}

	CLEANUP();
	tst_exit();
}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, CLEANUP);

	TEST_PAUSE;
}

/*
 * CLEANUP() - Performs one time CLEANUP for this test at
 * completion or premature exit
 */
void cleanup(void)
{
}
