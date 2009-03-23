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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/****************************************************************************
 *
 *    TEST IDENTIFIER	: getrlimit01
 *
 *    TEST TITLE	: test for checking functionality of getrlimit(2)
 *  $
 *    EXECUTED BY	: anyone
 *
 *    TEST CASE TOTAL	: 11
 *
 *    AUTHOR		: Suresh Babu V. <suresh.babu@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 * DESCRIPTION
 *      Verify that,
 *	getrlimit(2) call will be successful for all possible resource types.
 *
 * Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed
 *		Issue sys call failed to get resource limits.
 *      Otherwise,
 *		Issue sys call is successful and got resource limits.
 *
 *   Cleanup:
 *	Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  getrlimit01 [-c n] [-e] [-i n] [-I x] [-P x] [-p] [-t] [-h]
 *     where,  -c n  : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-p   : Pause for SIGUSR1 before starting.
 *		-t   : Turn on syscall timing.
 *		-h   : Display usage information.
 *
 ***************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "test.h"
#include "usctest.h"

extern int Tst_count;

static void cleanup(void);
static void setup(void);

char *TCID = "getrlimit01";

static struct rlimit rlim;
static struct test_t {
	int res;
	char *res_str;
} testcases[] = {
	{
	RLIMIT_CPU, "RLIMIT_CPU"}, {
	RLIMIT_FSIZE, "RLIMIT_FSIZE"}, {
	RLIMIT_DATA, "RLIMIT_DATA"}, {
	RLIMIT_STACK, "RLIMIT_STACK"}, {
	RLIMIT_CORE, "RLIMIT_CORE"}, {
	RLIMIT_RSS, "RLIMIT_RSS"}, {
	RLIMIT_NPROC, "RLIMIT_NPROC"}, {
	RLIMIT_NOFILE, "RLIMIT_NOFILE"}, {
	RLIMIT_MEMLOCK, "RLIMIT_MEMLOCK"}, {
	RLIMIT_AS, "RLIMIT_AS"}, {
	RLIMIT_LOCKS, "RLIMIT_LOCKS"}
};

int TST_TOTAL = sizeof(testcases) / sizeof(*testcases);

int main(int ac, char **av)
{
	int i;
	int lc;			/* loop counter */
	char *msg;		/* parse_opts() return message */

	/* Parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Do initial setup */
	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			/*
			 * Test the system call with different resoruce types
			 * with codes 0 to 10
			 */
			TEST(getrlimit(testcases[i].res, &rlim));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "getrlimit() failed to get %s "
					 "values. errno is %d",
					 testcases[i].res_str, TEST_ERRNO);
			} else {
				tst_resm(TPASS, "getrlimit() returned %d; "
					 "got %s values ",
					 TEST_ERRNO, testcases[i].res_str);
			}
		}
	}
	/* do cleanup and exit */
	cleanup();

	return 0;
}

/*
 * setup() - performs all one time setup for this test.
 */
void setup()
{
	/* capture the signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if the option was specified */
	TEST_PAUSE;
}

/*
 * cleanup()  - performs all one time cleanup for this test
 *		completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
