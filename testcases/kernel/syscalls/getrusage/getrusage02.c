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
 *    TEST IDENTIFIER	: getrusage02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Tests for error conditions
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that
 *	1) getrusage() fails with errno EINVAL when an invalid value
 *	   is given for who
 *	2) getrusage() fails with errno EFAULT when an invalid address
 *	   is given for usage
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  if call failed with expected errno,
 *		Test Passed
 *	  else
 *		Test Failed
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  getrusage02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f]
 * 			     [-p]
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
#include <sys/resource.h>
#include "test.h"

#ifndef RUSAGE_BOTH		/* Removed from user space on RHEL4 */
#define RUSAGE_BOTH (-2)	/* still works on SuSE      */
#endif /* so this is a work around */

static void setup();
static void cleanup();

char *TCID = "getrusage02";

static struct rusage usage;

struct test_cases_t {
	int who;
	struct rusage *usage;
	int exp_errno;
} test_cases[] = {
	{
	RUSAGE_BOTH, &usage, EINVAL},
#ifndef UCLINUX
	{
	RUSAGE_SELF, (struct rusage *)-1, EFAULT}
#endif
};

int TST_TOTAL = ARRAY_SIZE(test_cases);

int main(int ac, char **av)
{

	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(getrusage(test_cases[i].who, test_cases[i].usage));

			if (TEST_RETURN == -1 &&
			    TEST_ERRNO == test_cases[i].exp_errno)
				tst_resm(TPASS | TTERRNO,
					 "getrusage failed as expected");
			else
				tst_resm(TFAIL | TTERRNO,
					 "getrusage failed unexpectedly");
		}
	}

	cleanup();

	tst_exit();

}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

void cleanup(void)
{

}
