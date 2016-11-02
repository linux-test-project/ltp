/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
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
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: sched_setparam02
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Checks functionality for sched_setparam(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This test changes the scheduling priority for current process
 *	and verifies it by calling sched_getparam().
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Change scheduling policy to SCHED_FIFO
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  If scheduling priority is set properly,
 *		TEST passed
 *	  else
 *		TEST failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_setparam02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *			where,  -c n : Run n copies concurrently.
 *				-e   : Turn on errno logging.
 *				-h   : Show help screen
 *				-f   : Turn off functional testing
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *				-P x : Pause for x seconds between iterations.
 *				-t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <errno.h>
#include <sched.h>
#include "test.h"

#define FIFO_OR_RR_PRIO 5
#define OTHER_PRIO 0

static void setup();
static void cleanup();
static int verify_priority(int);

char *TCID = "sched_setparam02";

static struct sched_param param;
static struct sched_param param1 = { 1 };

static struct test_cases_t {
	char *desc;
	int policy;
	int priority;
} testcases[] = {
	{
	"Test with policy SCHED_FIFO", SCHED_FIFO, FIFO_OR_RR_PRIO}, {
	"Test with policy SCHED_RR", SCHED_RR, FIFO_OR_RR_PRIO}, {
	"Test with SCHED_OTHER", SCHED_OTHER, OTHER_PRIO}
};

int TST_TOTAL = sizeof(testcases) / sizeof(testcases[0]);

int main(int ac, char **av)
{

	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			if (i == 2) {
				param1.sched_priority = 0;
			} else {
				param1.sched_priority = 1;
			}
			if ((sched_setscheduler(0, testcases[i].policy,
						&param1)) == -1) {
				tst_brkm(TBROK, cleanup, "sched_setscheduler()"
					 "  failed");
			}
			param.sched_priority = testcases[i].priority;
			/*
			 * Call sched_setparam(2) with pid=0 sothat it will
			 * set the scheduling parameters for the calling process
			 */
			TEST(sched_setparam(0, &param));

			if ((TEST_RETURN == 0) && (verify_priority(i))) {
				tst_resm(TPASS, "%s Passed", testcases[i].desc);
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "%s Failed. sched_setparam()"
					 " returned %ld", testcases[i].desc,
					 TEST_RETURN);
			}
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
}

/*
 * verify_priority() -  This function checks whether the priority is
 *			set correctly
 */
int verify_priority(int i)
{
	struct sched_param p;

	if ((sched_getparam(0, &p)) == 0) {
		if (p.sched_priority == testcases[i].priority) {
			return 1;
		} else {
			tst_resm(TWARN, "sched_getparam() returned priority"
				 " value as %d", p.sched_priority);
			return 0;
		}
	}

	tst_resm(TWARN, "sched_getparam() failed");
	return 0;
}
