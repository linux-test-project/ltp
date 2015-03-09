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
 * Test Name: time02
 *
 * Test Description:
 *  Verify that time(2) returns the value of time in seconds since
 *  the Epoch and stores this value in the memory pointed to by the parameter.
 *
 * Expected Result:
 *  time() should return the time (seconds) since the Epoch and this value
 *  should be equal to the value stored in the specified parameter.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  time02 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
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
 *  None.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <stdint.h>

#include "test.h"

void setup();			/* setup function for the test */
void cleanup();			/* cleanup function for the test */

char *TCID = "time02";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;
	time_t tloc;		/* time_t variables for time(2) */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call time() to get the time in seconds$
		 * since Epoch.
		 */
		TEST(time(&tloc));

		/* Check return code from time(2) */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "time(0) Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (tloc == TEST_RETURN) {
				tst_resm(TPASS, "time() returned value "
					 "%ld, stored value %jd are same",
					 TEST_RETURN, (intmax_t) tloc);
			} else {
				tst_resm(TFAIL, "time() returned value "
					 "%ld, stored value %jd are "
					 "different", TEST_RETURN,
					 (intmax_t) tloc);
			}

		}
		tst_count++;	/* incr. TEST_LOOP counter */
	}

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{

}
