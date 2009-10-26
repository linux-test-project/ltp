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
 *    TEST IDENTIFIER	: ioperm02
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
 *	1) ioperm(2) returns -1 and sets errno to EINVAL for I/O port
 *	   address greater than 0x3ff.
 *	2) ioperm(2) returns -1 and sets errno to EPERM if the current
 *	   user is not the super-user.
 *
 * 	Setup:
 * 	  Setup signal handling.
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
 * ioperm02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

char *TCID = "ioperm02";	/* Test program identifier.    */

#if defined __i386__ || defined(__x86_64__)

#include <errno.h>
#include <unistd.h>
#include <sys/io.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

#define NUM_BYTES 3
#define TURN_ON 1
#define TURN_OFF 0
#define EXP_RET_VAL -1
#ifndef IO_BITMAP_BITS
#define IO_BITMAP_BITS 1024	/* set to default value since some H/W may not support 0x10000 even with a 2.6.8 kernel */
#define IO_BITMAP_BITS_16 65536
#endif

static void setup();
static int setup1(void);
static void cleanup1();
static void cleanup();

extern int Tst_count;		/* Test Case counter for tst_* routines */
static int exp_enos[] = { EINVAL, EPERM, 0 };

static char nobody_uid[] = "nobody";
struct passwd *ltpuser;

struct test_cases_t {
	long from;		/* starting port address */
	long num;		/* no. of bytes from starting address */
	int turn_on;
	char *desc;		/* test case description */
	int exp_errno;		/* expected error number */
};

int TST_TOTAL = 2;
struct test_cases_t *test_cases;

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			if (i == 1) {
				/* setup Non super-user for second test */
				if (setup1()) {
					/* setup1() failed, skip this test */
					continue;
				}
			}

			/* Test the system call */

			TEST(ioperm(test_cases[i].from,
				    test_cases[i].num, test_cases[i].turn_on));

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
			} else {
			}
		}

	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

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

/* cleanup1() - reset to super user for second test case */
void cleanup1()
{
	/* reset user as root */
	if (seteuid(0) == -1) {
		tst_brkm(TBROK, tst_exit, "Failed to set uid as root");
	}
}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check whether we are root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Must be root for this test!");
	}

	/* Check if "nobody" user id exists */
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, tst_exit, "\"nobody\" user id doesn't exist");
	}

	/*
	 * The value of IO_BITMAP_BITS (include/asm-i386/processor.h) changed
	 * from kernel 2.6.8 to permit 16-bits (65536) ioperm
	 *
	 * Ricky Ng-Adam, rngadam@yahoo.com
	 * */
	test_cases =
	    (struct test_cases_t *)malloc(sizeof(struct test_cases_t) * 2);
	test_cases[0].num = NUM_BYTES;
	test_cases[0].turn_on = TURN_ON;
	test_cases[0].desc = "Invalid I/O address";
	test_cases[0].exp_errno = EINVAL;
	test_cases[1].num = NUM_BYTES;
	test_cases[1].turn_on = TURN_ON;
	test_cases[1].desc = "Non super-user";
	test_cases[1].exp_errno = EPERM;
	if ((tst_kvercmp(2, 6, 8) < 0) || (tst_kvercmp(2, 6, 9) == 0)) {
		/*try invalid ioperm on 1022, 1023, 1024 */
		test_cases[0].from = (IO_BITMAP_BITS - NUM_BYTES) + 1;

		/*try get valid ioperm on 1021, 1022, 1023 */
		test_cases[1].from = IO_BITMAP_BITS - NUM_BYTES;
	} else {
		/*try invalid ioperm on 65534, 65535, 65536 */
		test_cases[0].from = (IO_BITMAP_BITS_16 - NUM_BYTES) + 1;

		/*try valid ioperm on 65533, 65534, 65535 */
		test_cases[1].from = IO_BITMAP_BITS_16 - NUM_BYTES;
	}

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;

}				/* End setup() */

void cleanup()
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();

}				/* End cleanup() */

#else /* __i386__ */

#include "test.h"
#include "usctest.h"

int TST_TOTAL = 0;		/* Total number of test cases. */

int main()
{
	tst_resm(TPASS,
		 "LSB v1.3 does not specify ioperm() for this architecture.");
	tst_exit();
	return 0;
}

#endif /* __i386__ */
