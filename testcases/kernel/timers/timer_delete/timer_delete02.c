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
 *    TEST IDENTIFIER	: timer_delete02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for timer_delete(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *     This is a Phase I test for the timer_delete(2) system call.
 *     It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 Create a POSIX timer
 *	 Execute system call
 *	 Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	 Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * timer_delete02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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

char *TCID = "timer_delete02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int main(int ac, char **av)
{
	int lc;
	kernel_timer_t timer_id;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* Create a Posix timer */
		if (ltp_syscall(__NR_timer_create, CLOCK_REALTIME, NULL,
			&timer_id) < 0) {
			tst_count = TST_TOTAL;
			tst_brkm(TBROK | TERRNO, cleanup,
				 "timer_delete can't be tested because "
				 "timer_create failed");
		}
		TEST(ltp_syscall(__NR_timer_delete, timer_id));
		tst_resm((TEST_RETURN == 0 ? TPASS : TFAIL | TTERRNO),
			 "%s", (TEST_RETURN == 0 ? "passed" : "failed"));
	}

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
