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
 * NAME
 *	sched_getscheduler02.C
 *
 * DESCRIPTION
 *	To check for the errno ESRCH
 *
 * ALGORITHM
 *	Pass an invalid pid to sched_getscheduler() and test for ESRCH.
 *
 * USAGE:  <for command-line>
 *  sched_getscheduler02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * RESTRICTION
 *	None
 */

#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "test.h"

char *TCID = "sched_getscheduler02";
int TST_TOTAL = 1;

static pid_t unused_pid;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		TEST(sched_getscheduler(unused_pid));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "sched_getscheduler(2) passed "
				 "unexpectedly");
			continue;
		}

		if (errno != ESRCH) {
			tst_resm(TFAIL, "Expected ESRCH, got %d", errno);
		} else {
			tst_resm(TPASS, "call failed with ESRCH");
		}
	}
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	unused_pid = tst_get_unused_pid(cleanup);

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
