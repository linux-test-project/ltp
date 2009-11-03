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
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
 /*******************************************************************
 *
 *    TEST IDENTIFIER   : clone04
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : test for checking error conditions for clone(2)
 *
 *    TEST CASE TOTAL   : 2
 *
 *    AUTHOR            : Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 * DESCRIPTION
 *	Verify that,
 *      clone(2) returns -1 and sets errno to EINVAL if
 *	child stack is set to a zero value(NULL)
 *
 * ALGORITHM
 * Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if (system call failed (return=-1)) &
 *			   (errno set == expected errno)
 *              Issue sys call fails with expected return value and errno.
 *   Otherwise,
 *      Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *        Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  clone04 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *		where,  -c n : Run n copies concurrently.
 *			-e   : Turn on errno logging.
 *			-h   : Show help screen
 *			-f   : Turn off functional testing
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-p   : Pause for SIGUSR1 before starting
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 *********************************************************************/

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#include <sched.h>
#include <errno.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"
#include "clone_platform.h"

static void cleanup(void);
static void setup(void);
static int child_fn();

char *TCID = "clone04";
extern int Tst_count;
void *child_stack;

static int exp_enos[] = { EINVAL, 0 };	/* 0 terminated list of *
					 * expected errnos */
static struct test_case_t {
	int (*child_fn) ();
	void **child_stack;
	int exp_errno;
	char err_desc[10];
} test_cases[] = {
	{
child_fn, NULL, EINVAL, "EINVAL"},};

int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{
	int lc, ind;		/* loop counter */
	char *msg;		/* message returned from parse_opts */
	void *test_stack;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			if (test_cases[ind].child_stack == NULL) {
				test_stack = (void *)NULL;
			} else if (*test_cases[ind].child_stack == NULL) {
				tst_resm(TWARN, "Can not allocate stack for"
					 "child, skipping test case");
				continue;
			} else {
				test_stack = child_stack;
			}

			/*
			 * call the system call with the TEST() macro
			 */
			TEST(ltp_clone(0,test_cases[ind].child_fn, NULL,
					CHILD_STACK_SIZE, test_stack));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == test_cases[ind].exp_errno)) {
				tst_resm(TPASS, "expected failure; Got %s",
					 test_cases[ind].err_desc);
			} else {
				tst_resm(TFAIL|TTERRNO, "Call failed to produce expected error; "
					 "expected errno %d and result -1; got result %ld",
					 test_cases[ind].exp_errno, TEST_RETURN);
			}
			TEST_ERROR_LOG(TEST_ERRNO);
		}
	}

	cleanup();

	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Allocate stack for child */
	child_stack = (void *)malloc(CHILD_STACK_SIZE);

}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup(void)
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	free(child_stack);

	/* exit with return code appropriate for results */
	tst_exit();
}

/*
 * child_fn()	- Child function
 */
int child_fn()
{
	exit(1);
}
