/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: stime01
 *
 * Test Description:
 *  Verify that the system call stime() successfully sets the system's idea
 *  of date and time if invoked by "root" user.
 *
 * Expected Result:
 *  stime() should succeed to set the system data/time to the specified time.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *   	Verify the Functionality of system call
 *      if successful,
 *      	Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  stime01 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 *  This test should be run by 'super-user' (root) only.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>

#include "test.h"

#define INCR_TIME	30	/* increment in the system's current time */

#define BASH_CLOCK

char *TCID = "stime01";
int TST_TOTAL = 1;
struct timeval real_time_tv, pres_time_tv;
time_t new_time;

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/*
		 * ``Break`` the clock.
		 *
		 * This is being done inline here so that the offset is
		 * automatically reset based on the elapsed time, and not a
		 * fixed time sampled once in setup.
		 *
		 * The big assumption here is the clock state isn't super
		 * fubared if so, the executing party needs to go fix their
		 * RTC's battery, or they have more pressing issues to attend
		 * to as far as clock skew is concerned :P.
		 */
		if (gettimeofday(&real_time_tv, NULL) < 0) {
			tst_brkm(TBROK | TERRNO, NULL,
				 "failed to get current time via gettimeofday(2)");
		}

		/* Get the system's new time */
		new_time = real_time_tv.tv_sec + INCR_TIME;

		tst_count = 0;

		/*
		 * Invoke stime(2) to set the system's time to the specified
		 * new_time.
		 */
		if (stime(&new_time) < 0) {
			tst_resm(TFAIL | TERRNO, "stime(%ld) failed", new_time);
		} else {

			/*
			 * Get the system's current time after call
			 * to stime().
			 */
			if (gettimeofday(&pres_time_tv, NULL) < 0) {
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "time() failed to get "
					 "system's time after stime");
			}

			/* Now do the actual verification */
			switch (pres_time_tv.tv_sec - new_time) {
			case 0:
			case 1:
				tst_resm(TINFO, "pt.tv_sec: %ld",
					 pres_time_tv.tv_sec);
				tst_resm(TPASS, "system time was set "
					 "to %ld", new_time);
				break;
			default:
				tst_resm(TFAIL, "system time was not "
					 "set to %ld (time is "
					 "actually: %ld)",
					 new_time, pres_time_tv.tv_sec);
			}

			if (settimeofday(&real_time_tv, NULL) < 0) {
				tst_resm(TBROK | TERRNO,
					 "failed to restore time to original "
					 "value; system clock may need to be "
					 "fixed manually");
			}

		}

	}

	cleanup();
	tst_exit();

}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 *  Get the current time and system's new time to be set in the test.
 */
void setup(void)
{
	tst_require_root();

	TEST_PAUSE;

}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup(void)
{

	/* Restore the original system time. */
	if (settimeofday(&real_time_tv, NULL) != 0) {
		tst_resm(TBROK | TERRNO, "failed to restore time to original "
			 "value; system clock may need to be "
			 "fixed manually");
	}

}
