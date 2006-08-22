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
 *    TEST IDENTIFIER	: timer_settime03
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Test checking for basic error conditions for
 *    			  timer_settime(2)
 *
 *    TEST CASE TOTAL	: 6
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *    	This test case check whether timer_settime(2) returns appropriate error
 *    	value for invalid parameter
 *
 * 	Setup:
 *	 Setup signal handling.
 *	 Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 setup the individual test.
 *	 Execute system call with invalid parameters.
 *	 Check return code, if system call fails with errno == expected errno
 * 	 	Issue syscall passed with expected errno
 *	 Otherwise, Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 	 Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * timer_settime03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
#include <unistd.h>

#include "test.h"
#include "usctest.h"
#include "common_timers.h"

static void setup();
static void cleanup();
static void setup_test(int option);

char *TCID = "timer_settime03"; 	/* Test program identifier.    */
int TST_TOTAL;				/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

static struct itimerspec new_set, old_set, *old_temp, *new_temp;
static timer_t timer, tim;

static int exp_enos[] = {EINVAL, EFAULT, 0};

static struct test_case_t {
	char *err_desc;		/* error description */
	int  exp_errno;		/* expected error number */
	char *exp_errval;	/* Expected errorvalue string */
} testcase[] = {
	{"Invalid parameter", EINVAL, "EINVAL"},	/* New setting null */
	{"Invalid parameter", EINVAL, "EINVAL"},	/* tv_nsec < 0 */
	{"Invalid parameter", EINVAL, "EINVAL"},	/* nsec > NSEC/SEC */
	{"Invalid parameter", EINVAL, "EINVAL"},	/* Invalid timerid */
	{"Bad address", EFAULT, "EFAULT"},		/* bad newsetting * */
	{"Bad address", EFAULT, "EFAULT"}		/* bad oldsetting * */
};

int
main(int ac, char **av)
{
	int lc, i;			/* loop counter */
	char *msg;			/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL))
		!= (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	TST_TOTAL = sizeof(testcase) / sizeof(testcase[0]);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/* Set up individual tests */
			setup_test(i);
			TEST(timer_settime(tim, 0, new_temp, old_temp));

			if (TEST_ERRNO == ENOSYS) {
			/* system call not implemented */
				Tst_count = TST_TOTAL;
				perror("timer_settime");
				tst_brkm(TBROK, cleanup, "");
			}

			/* check return code */
			if ((TEST_RETURN == -1) && (TEST_ERRNO == testcase[i].
						exp_errno)) {
				tst_resm(TPASS, "timer_create(2) expected"
						" failure; Got errno - %s : %s"
						, testcase[i].exp_errval,
						testcase[i].err_desc);
			} else {
				tst_resm(TFAIL, "timer_create(2) failed to"
						" produce expected error; %d"
						" , errno : %s and got %d",
						testcase[i].exp_errno,
						testcase[i].exp_errval,
						TEST_ERRNO);
			} /* end of else */

			TEST_ERROR_LOG(TEST_ERRNO);
		}	/* End of TEST CASE LOOPING */
	}	/* End for TEST_LOOPING */

	/* Clean up and exit */
	cleanup();

	/* NOTREACHED */
	return 0;
}

/* This function sets up individual tests */
void
setup_test(int option)
{
	switch (option) {
		case 0:
			/* Pass NULL structure as new setting */
			new_temp = (struct itimerspec *) NULL;
			tim = timer;
			old_temp = &old_set;
			break;
		case 1:
			/* Make tv_nsec value less than 0 */
			new_set.it_value.tv_nsec = -1;
			new_set.it_value.tv_sec = 5;
			new_set.it_interval.tv_sec = 0;
			new_set.it_interval.tv_nsec = 0;
			new_temp = &new_set;
			break;
		case 2:
			/* Make tv_nsec more than NSEC_PER_SEC */
			new_set.it_value.tv_nsec = NSEC_PER_SEC + 1;
			break;
		case 3:
			/* make timer_id invalid */
			tim = (timer_t)-1;
			new_set.it_value.tv_nsec = 0;
			break;
		case 4:
			/* make new_setting a bad pointer */
			tim = timer;
			new_temp = (struct itimerspec *) -1;
			break;
		case 5:
			/* make old_setting a bad pointer */
			new_temp = &new_set;
			old_temp = (struct itimerspec *) -1;
			break;
	}
}

/* setup() - performs all ONE TIME setup for this test */
void
setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	if (timer_create(CLOCK_REALTIME, NULL, &timer) < 0) {
		if (errno == ENOSYS) {
			/* System call not implemented */
			TST_TOTAL = 1;
			perror("timer_create");
			tst_brkm(TBROK, tst_exit, "");
		}
		tst_brkm(TBROK, tst_exit, "Timer create failed. Cannot"
				" setup test");
	}

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);
	/* Pause if that option was specified */
	TEST_PAUSE;

}	/* End setup() */

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */

void
cleanup()
{
	/*
	* print timing stats if that option was specified.
	* print errno log if that option was specified.
	*/
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}	/* End cleanup() */
