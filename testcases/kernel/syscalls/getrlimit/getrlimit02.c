/*
 *
 *   Copyright (c) Wipro Technologies, 2002. All Rights Reserved.
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

/****************************************************************************
 *
 *    TEST IDENTIFIER	: getrlimit02
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: test for checking error conditions for getrlimit(2)
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Suresh Babu V. <suresh.babu@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 * DESCRIPTION
 *      Verify that,
 *   1) getrlimit(2) returns -1 and sets errno to EFAULT if an invalid
 *	address is given for address parameter.
 *   2) getrlimit(2) returns -1 and sets errno to EINVAL if an invalid
 *	resource type (RLIM_NLIMITS is a out of range resource type) is
 *	passed.
 *
 * Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed and errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *      Otherwise,
 *		Issue sys call failed to produce expected error.
 *
 *   Cleanup:
 *	Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  getrlimit02 [-c n] [-e] [-i n] [-I x] [-P x] [-p] [-t] [-h]
 *     where,  -c n  : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-p   : Pause for SIGUSR1 before startingt
 *		-t   : Turn on syscall timing.
 *		-h   : Display usage information.
 *
 ***************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/resource.h>
#include "test.h"

#define RLIMIT_TOO_HIGH 1000

char *TCID = "getrlimit02";

static void cleanup(void);
static void setup(void);

static struct rlimit rlim;
static struct test_case_t {
	int exp_errno;		/* Expected error no            */
	char *exp_errval;	/* Expected error value string  */
	struct rlimit *rlim;	/* rlimit structure             */
	int res_type;		/* resource type                */

} testcases[] = {
#ifndef UCLINUX
	/* Skip since uClinux does not implement memory protection */
	{
	EFAULT, "EFAULT", (void *)-1, RLIMIT_NOFILE},
#endif
	{
	EINVAL, "EINVAL", &rlim, RLIMIT_TOO_HIGH}
};

int TST_TOTAL = ARRAY_SIZE(testcases);

int main(int ac, char **av)
{
	int i;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	/* Do initial setup */
	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			/*
			 * Test the system call.
			 */
			TEST(getrlimit(testcases[i].res_type,
				       testcases[i].rlim));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == testcases[i].exp_errno)) {
				tst_resm(TPASS, "expected failure; got %s",
					 testcases[i].exp_errval);
			} else {
				tst_resm(TFAIL, "call failed to produce "
					 "expected error;  errno: %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
		}
	}
	/* do cleanup and exit */
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all one time setup for this test.
 */
void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if the option was specified */
	TEST_PAUSE;
}

/*
 * cleanup()  - performs all one time cleanup for this test
 *		completion or premature exit.
 */
void cleanup(void)
{

}
