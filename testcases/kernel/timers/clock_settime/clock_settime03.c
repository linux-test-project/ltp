/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
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
/**************************************************************************
 *
 *	TEST IDENTIFIER	: clock_settime03
 *
 *	EXECUTED BY	: root / superuser
 *
 *	TEST TITLE	: Test checking for basic error conditions for
 *				  clock_settime(2)
 *
 *	TEST CASE TOTAL	: 6
 *
 *	AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *	SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *	DESCRIPTION
 *		This test case check whether clock_settime(2) returns appropriate error
 *		value for invalid parameter
 *
 * 	Setup:
 *	 Setup signal handling.
 *	 Pause for SIGUSR1 if option specified.
 *	 Setup the value for struct timespec variable
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 If it is the first test case
 *	 	make temp a bad pointer
 *	 If it is the last test case
 *	 	change the UID to non-previleged one.
 *	 Otherwise pass defined struct timespec variable to temp
 *	 Execute system call with invalid parameter
 *	 Check return code, if system call fails with errno == expected errno
 * 	 	Issue syscall passed with expected errno
 *	 Otherwise, Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 	 Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * clock_settime03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
 * where:
 * 	-c n : run n copies simultaneously
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 * RESTRICTIONS:
 * None
 *****************************************************************************/

#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>

#include "test.h"
#include "usctest.h"
#include "common_timers.h"

static void setup();
static int setup_test(int option);

char *TCID = "clock_settime03"; /* Test program identifier.	*/
int TST_TOTAL;			/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
static int exp_enos[] = { EINVAL, EFAULT, EPERM, 0 };
char nobody_uid[] = "nobody";
struct passwd *ltpuser;
static struct timespec spec, *temp, saved;

clockid_t clocks[] = {
	CLOCK_REALTIME,
	CLOCK_MONOTONIC,
	MAX_CLOCKS,
	MAX_CLOCKS + 1,
	CLOCK_REALTIME,
	CLOCK_REALTIME,
	CLOCK_REALTIME,
	CLOCK_PROCESS_CPUTIME_ID,
	CLOCK_THREAD_CPUTIME_ID
};

int testcase[9] = {
	EFAULT,	/* tp bad		*/
	EINVAL,	/* CLOCK_MONOTONIC	*/
	EINVAL,	/* MAX_CLOCKS		*/
	EINVAL,	/* MAX_CLOCKS + 1	*/
	EINVAL,	/* Invalid timespec	*/
	EINVAL,	/* NSEC_PER_SEC + 1	*/
	EPERM	/* non-root user	*/
};

int
main(int ac, char **av)
{
	int lc, i;	/* loop counter */
	char *msg;	/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL))
		!= (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	TST_TOTAL = sizeof(testcase) / sizeof(testcase[0]);

	/* PROCESS_CPUTIME_ID & THREAD_CPUTIME_ID are not supported on
	 * kernel versions lower than 2.6.12
	 */
	if((tst_kvercmp(2, 6, 12)) < 0) {
		testcase[7] = EINVAL;
		testcase[8] = EINVAL;
	} else {
		testcase[7] = EFAULT;
		testcase[8] = EFAULT;
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (setup_test(i) < 0) {
				continue;
			}

			TEST(syscall(__NR_clock_settime, clocks[i], temp));

			/* Change the UID back to root */
			if (i == TST_TOTAL - 1) {
				if (seteuid(0) == -1) {
					tst_brkm(TBROK | TERRNO, cleanup,
						"Failed to set the effective "
						"uid to root");
				}
			}

			/* check return code */
			if (TEST_RETURN == -1 && TEST_ERRNO == testcase[i]) {
				tst_resm(TPASS | TTERRNO,
					"clock_settime(2) got expected "
					"failure.");
			} else {
				tst_resm(TFAIL | TTERRNO,
					"clock_settime(2) failed to produce "
					"expected error (return code = %ld)",
					TEST_RETURN);
				/* Restore the clock to its previous state. */
				if (TEST_RETURN == 0) {
					if (syscall(__NR_clock_settime,
						CLOCK_REALTIME,	&saved) < 0) {
						tst_resm(TWARN | TERRNO,
							"FATAL: could not set "
							"the clock!");
					}
				}
			} /* end of else */

		}	/* End of TEST CASE LOOPING */

	}	/* End for TEST_LOOPING */

	/* Clean up and exit */
	cleanup();
	tst_exit();
}

/* This function sets up the individual test */
static int
setup_test(int option)
{
	/* valid timespec */
	spec = saved;
	temp = &spec;

	/* error sceanrios */
	switch (option) {
	case 0:
		/* Make tp argument bad pointer */
		temp = (struct timespec *) -1;
		break;
	case 4:
		/* Make the parameter of timespec invalid */
		spec.tv_nsec = -1;
		break;
	case 5:
		/* Make the parameter of timespec invalid */
		spec.tv_nsec = NSEC_PER_SEC + 1;
		break;
	case 6:
		/* change the User to non-root */
		spec.tv_nsec = 0;
		if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
			tst_resm(TWARN, "\"nobody\" user not present."
					" skipping test");
			return -1;
		}
		if (seteuid(ltpuser->pw_uid) == -1) {
			tst_resm(TWARN | TERRNO,
				"seteuid failed to set the effective "
				"uid to %d (nobody)",
				ltpuser->pw_uid);
			return -1;
		}
		break;
	case 7:
	case 8:
		/* Make tp argument bad pointer */
		if (tst_kvercmp(2, 6, 12) >= 0)
			temp = (struct timespec *) -1;
	}
	return 0;
}

/* setup() - performs all ONE TIME setup for this test */
static void
setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Check whether we are root*/
	if (geteuid() != 0)
		tst_brkm(TBROK, tst_exit, "Test must be run as root");

	if (syscall(__NR_clock_gettime, CLOCK_REALTIME, &saved) < 0)
		tst_brkm(TBROK, tst_exit, "Clock gettime failed");

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);
	spec.tv_sec = 1;
	spec.tv_nsec = 0;

	/* Pause if that option was specified */
	TEST_PAUSE;
}	/* End setup() */

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */

static void
cleanup(void)
{
	/*
	* print timing stats if that option was specified.
	* print errno log if that option was specified.
	*/
	TEST_CLEANUP;

}	/* End cleanup() */
