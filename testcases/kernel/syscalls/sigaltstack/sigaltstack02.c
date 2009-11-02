/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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

/*
 * Test Name: sigalstack02
 *
 * Test Description:
 *  Verify that,
 *   1. sigaltstack() fails and sets errno to EINVAL when "ss_flags" field
 *      pointed to by 'ss' contains invalid flags.
 *   2. sigaltstack() fails and sets errno to ENOMEM when the size of alternate
 *      stack area is less than MINSIGSTKSZ.
 *
 * Expected Result:
 *  sigaltstack() should fail with return value -1 and set expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  sigaltstack02 [-c n] [-e] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 *  This test should be executed by super-user (root) only.
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ucontext.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

#define INVAL_FLAGS	9999

char *TCID = "sigaltstack02";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EINVAL, ENOMEM, 0 };

stack_t sigstk;			/* signal stack storing struct. */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

struct test_case_t {		/* test case struct. to hold diff. test.conds */
	int flag;
	int size;
	char *desc;
	int exp_errno;
} Test_cases[] = {
	{
	INVAL_FLAGS, SIGSTKSZ, "Invalid Flag value", EINVAL},
#ifdef __ia64__
	{
	0, (131027 - 1), "alternate stack is < MINSIGSTKSZ", ENOMEM},
#else
	{
	0, (MINSIGSTKSZ - 1), "alternate stack is < MINSIGSTKSZ", ENOMEM},
#endif
	{
	0, 0, NULL, 0}
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *test_desc;	/* test specific error message */
	int ind;		/* counter to test different test conditions */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}

	/* Perform global setup for test */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			sigstk.ss_size = Test_cases[ind].size;
			sigstk.ss_flags = Test_cases[ind].flag;
			test_desc = Test_cases[ind].desc;

			/* Verify sigaltstack() fails and sets errno */
			TEST(sigaltstack(&sigstk, (stack_t *) 0));

			/* Check return code from sigaltstack() */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				/*
				 * Perform functional verification if test
				 * executed without (-f) option.
				 */
				if (STD_FUNCTIONAL_TEST) {
					if (TEST_ERRNO ==
					    Test_cases[ind].exp_errno) {
						tst_resm(TPASS, "stgaltstack() "
							 "fails, %s, errno:%d",
							 test_desc, TEST_ERRNO);
					} else {
						tst_resm(TFAIL, "sigaltstack() "
							 "fails, %s, errno:%d, "
							 "expected errno:%d",
							 test_desc, TEST_ERRNO,
							 Test_cases[ind].
							 exp_errno);
					}
				} else {
					tst_resm(TPASS, "Call returned -1 as "
						 "expected.");
				}
			} else {
				tst_resm(TFAIL, "sigaltstack() returned %ld, "
					 "expected -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}		/* End of TEST CASE LOOPING. */
		Tst_count++;	/* incr. TEST_LOOP counter */
	}			/* End of TEST CASE LOOPING. */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();
	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 * Allocate memory for the alternative stack.
 */
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Allocate memory for the alternate stack */
	if ((sigstk.ss_sp = (void *)malloc(SIGSTKSZ)) == NULL) {
		tst_brkm(TFAIL, cleanup,
			 "could not allocate memory for the alternate stack");
	 /*NOTREACHED*/}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  Free the memory allocated for alternate stack.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	free(sigstk.ss_sp);

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
