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
/**************************************************************************
 *
 *    TEST IDENTIFIER	: mlockall02
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Test for checking basic error conditions for
 *    			   mlockall(2)
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 * 	Check for basic errors returned by mount(2) system call.
 *$
 * 	Verify that mount(2) returns -1 and sets errno to
 *
 *	1) ENOMEM - If process exceed maximum  number of locked pages.
 *	2) EPERM  - If not super user
 *	3) EINVAL - Unknown flags were specified.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Do necessary setup for each test.
 *	  Execute system call
 *	  Check return code, if system call failed and errno == expected errno
 *		Issue sys call passed with expected return value and errno.
 *	  Otherwise,
 *		Issue sys call failed to produce expected error.
 *	  Do cleanup for each test.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  mlockall02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *		where,
 *			-c n : Run n copies concurrently
 *			-e   : Turn on errno logging.
 *			-h   : Show this help screen
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-p   : Pause for SIGUSR1 before starting
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 * RESTRICTIONS
 *	Test must run as root.
 *****************************************************************************/
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/mman.h>
#include "test.h"
#include "safe_macros.h"
#include <sys/resource.h>

void setup();
int setup_test(int);
void cleanup_test(int);
void cleanup();

char *TCID = "mlockall02";
int TST_TOTAL = 3;

struct test_case_t {
	int flag;		/* flag value                   */
	int error;		/* error description            */
	char *edesc;		/* Expected error no            */
} TC[] = {
	{
	MCL_CURRENT, ENOMEM, "Process exceeds max locked pages"}, {
	MCL_CURRENT, EPERM, "Not a superuser"}, {
	0, EINVAL, "Unknown flag"}
};

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (setup_test(i)) {
				tst_resm(TFAIL, "mlockall() Failed while setup "
					 "for checking error %s", TC[i].edesc);
				continue;
			}

			TEST(mlockall(TC[i].flag));

			/* check return code */
			if (TEST_RETURN == -1) {
				if (TEST_ERRNO != TC[i].error)
					tst_brkm(TFAIL, cleanup,
						 "mlock() Failed with wrong "
						 "errno, expected errno=%s, "
						 "got errno=%d : %s",
						 TC[i].edesc, TEST_ERRNO,
						 strerror(TEST_ERRNO));
				else
					tst_resm(TPASS,
						 "expected failure - errno "
						 "= %d : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
			} else {
				if (i <= 1)
					tst_resm(TCONF,
						 "mlockall02 did not BEHAVE as expected.");
				else
					tst_brkm(TFAIL, cleanup,
						 "mlock() Failed, expected "
						 "return value=-1, got %ld",
						 TEST_RETURN);
			}
			cleanup_test(i);
		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	return;
}

int setup_test(int i)
{
	struct rlimit rl;
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;

	switch (i) {
	case 0:
		rl.rlim_max = 10;
		rl.rlim_cur = 7;

		if (setrlimit(RLIMIT_MEMLOCK, &rl) != 0) {
			tst_resm(TWARN, "setrlimit failed to set the "
				 "resource for RLIMIT_MEMLOCK to check "
				 "for mlockall error %s\n", TC[i].edesc);
			return 1;
		}
		ltpuser = getpwnam(nobody_uid);
		if (seteuid(ltpuser->pw_uid) == -1) {
			tst_brkm(TBROK, cleanup, "seteuid() "
				"failed to change euid to %d "
				"errno = %d : %s",
				ltpuser->pw_uid, TEST_ERRNO,
				strerror(TEST_ERRNO));
				return 1;
		}
		return 0;
	case 1:
		rl.rlim_max = 0;
		rl.rlim_cur = 0;
		if (setrlimit(RLIMIT_MEMLOCK, &rl) != 0) {
			tst_resm(TWARN, "setrlimit failed to "
				"set the resource for "
				"RLIMIT_MEMLOCK to check for "
				"mlockall error %s\n", TC[i].edesc);
				return 1;
		}
		ltpuser = getpwnam(nobody_uid);
		if (seteuid(ltpuser->pw_uid) == -1) {
			tst_brkm(TBROK, cleanup, "seteuid() failed to "
				 "change euid to %d errno = %d : %s",
				 ltpuser->pw_uid, TEST_ERRNO,
				 strerror(TEST_ERRNO));
			return 1;
		}
		return 0;
	}
	return 0;
}

void cleanup_test(int i)
{
	struct rlimit rl;

	switch (i) {
	case 0:
		seteuid(0);

		rl.rlim_max = -1;
		rl.rlim_cur = -1;

		if (setrlimit(RLIMIT_MEMLOCK, &rl) != 0) {
			tst_brkm(TFAIL, cleanup,
				 "setrlimit failed to reset the "
				 "resource for RLIMIT_MEMLOCK while "
				 "checking for mlockall error %s\n",
				 TC[i].edesc);
		}
		return;
	case 1:
		SAFE_SETEUID(cleanup, 0);
		return;

	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
	return;
}
