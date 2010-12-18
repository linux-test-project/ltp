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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER	: iopl02
 *
 *    EXECUTED BY	: superuser
 *
 *    TEST TITLE	: Tests for error conditions
 *
 *    TEST CASE TOTAL	: 2
 *
 *    AUTHOR		: Subhab Biwas <subhabrata.biswas@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify that
 *	1) iopl(2) returns -1 and sets errno to EINVAL for privilege
 *	   level greater than 3.
 *	2) iopl(2) returns -1 and sets errno to EPERM if the current
 *	   user is not the super-user.
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Test caller is superuser
 *	  Set expected errnos for logging
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code and error number, if matching,
 *		     Issue PASS message
 *	  Otherwise,
 *		     Issue FAIL message
 *	  Perform testcase specific cleanup (if needed)
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * iopl02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

char *TCID = "iopl02";		/* Test program identifier.    */

#if defined __i386__ || defined(__x86_64__)

#include <errno.h>
#include <unistd.h>
#include <sys/io.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

#define INVALID_LEVEL 4		/* Invalid privilege level */
#define EXP_RET_VAL -1

static void setup();
static int setup1(void);
static void cleanup1();
static void cleanup();

static int exp_enos[] = { EINVAL, EPERM, 0 };

static char nobody_uid[] = "nobody";
struct passwd *ltpuser;

struct test_cases_t {
	int level;		/* I/O privilege level */
	char *desc;		/* test case description */
	int exp_errno;		/* expected error number */
} test_cases[] = {
	{
	INVALID_LEVEL, "Invalid privilege level", EINVAL}, {
	1, "Non super-user", EPERM}
};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{

	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			if (i == 1) {
				/* setup Non super-user for second test */
				if (setup1()) {
					/* setup1() failed, skip this test */
					continue;
				}
			}

			/*
			 * Call iopl(2)
			 */
			TEST(iopl(test_cases[i].level));

			if ((TEST_RETURN == EXP_RET_VAL) &&
			    (TEST_ERRNO == test_cases[i].exp_errno)) {
				tst_resm(TPASS, "Expected failure for %s, "
					 "errno: %d", test_cases[i].desc,
					 TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "Unexpected results for %s ; "
					 "returned %ld (expected %d), errno %d "
					 "(expected errno  %d)",
					 test_cases[i].desc,
					 TEST_RETURN, EXP_RET_VAL,
					 TEST_ERRNO, test_cases[i].exp_errno);
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (i == 1) {
				/* revert back to super user */
				cleanup1();
			}

		}
	}

	/* cleanup and exit */
	cleanup();

	tst_exit();

}

/* setup1() - set up non-super user for second test case */
int setup1(void)
{
	/* switch to "nobody" user */
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TWARN, "Failed to set effective"
			 "uid to %d", ltpuser->pw_uid);
		return 1;
	}
	return 0;
}

/* cleanup1() - reset to super user for first test case */
void cleanup1()
{
	/* reset user as root */
	if (seteuid(0) == -1) {
		tst_brkm(TBROK, NULL, "Failed to set uid as root");
	}
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check whether we are root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must be root for this test!");
	}

	/* Check if "nobody" user id exists */
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, NULL, "\"nobody\" user id doesn't exist");
	}

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

}

/*
 *cleanup() -  performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup()
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}

#else /* __i386__ */

#include "test.h"
#include "usctest.h"

int TST_TOTAL = 0;		/* Total number of test cases. */

int main()
{
	tst_resm(TPASS,
		 "LSB v1.3 does not specify iopl() for this architecture.");
	tst_exit();
	tst_exit();
}

#endif /* __i386__ */