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
 *	sched_getscheduler01.C
 *
 * DESCRIPTION
 *	Testcase to check sched_getscheduler() returns correct return value
 *
 * ALGORTIHM
 *	Call sched_setcheduler() to set the scheduling policy of the current
 *	process. Then call sched_getscheduler() to ensure that this is same
 *	as what set by the previous call to sched_setscheduler().
 *
 *	Use SCHED_RR, SCHED_FIFO, SCHED_OTHER as the scheduling policies for
 *	sched_setscheduler().
 *
 * USAGE:  <for command-line>
 *  sched_getscheduler01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * RESTRICTION
 *	Must run test as root.
 */

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include "test.h"

char *TCID = "sched_getscheduler01";
int TST_TOTAL = 3;

void setup(void);
void cleanup(void);

struct test_case_t {
	int prio;
	int policy;
} TC[] = {
	/* set scheduling policy to SCHED_RR */
	{
	1, SCHED_RR},
	    /* set scheduling policy to SCHED_OTHER */
	{
	0, SCHED_OTHER},
	    /* set scheduling policy to SCHED_FIFO */
	{
	1, SCHED_FIFO}
};

int main(int ac, char **av)
{
	int lc;
	int i;
	struct sched_param param;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			param.sched_priority = TC[i].prio;

			if (sched_setscheduler(0, TC[i].policy, &param) == -1)
				tst_brkm(TBROK, cleanup,
					 "sched_setscheduler failed");

			TEST(sched_getscheduler(0));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "call failed unexpectedly");
				continue;
			}

			if (TEST_RETURN != TC[i].policy)
				tst_resm(TFAIL,
					 "policy value returned is not "
					 "correct");
			else
				tst_resm(TPASS,
					 "policy value returned is correct");
		}
	}

	cleanup();
	tst_exit();
}

void setup(void)
{

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
}
