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
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: sched_getparam01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for sched_getparam(2)
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
 *	This is a Phase I test for the sched_getparam(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  sched_getparam01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

static void setup();
static void cleanup();

char *TCID = "sched_getparam01";
int TST_TOTAL = 1;

static struct sched_param param;

int main(int ac, char **av)
{

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		param.sched_priority = 100;

		/*
		 * Call sched_getparam(2) with pid=0 sothat it will
		 * get the scheduling parameters for the calling process
		 */
		TEST(sched_getparam(0, &param));

		/*
		 * Check return code & priority. For normal process,
		 * scheduling policy is SCHED_OTHER. For this scheduling
		 * policy, only allowed priority value is 0. So we should
		 * get 0 for priority value
		 */
		if ((TEST_RETURN == 0) && (param.sched_priority == 0)) {
			tst_resm(TPASS, "sched_getparam() returned %ld",
				 TEST_RETURN);
		} else {
			tst_resm(TFAIL, "Test Failed, sched_getparam()"
				 "returned %ld, errno = %d : %s; returned "
				 "process priority value is %d", TEST_RETURN,
				 TEST_ERRNO, strerror(TEST_ERRNO),
				 param.sched_priority);
		}
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
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
}
