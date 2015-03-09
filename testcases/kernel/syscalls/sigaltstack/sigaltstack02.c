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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#define INVAL_FLAGS	9999

char *TCID = "sigaltstack02";
int TST_TOTAL = 2;

stack_t sigstk;			/* signal stack storing struct. */

void setup(void);			/* Main setup function of test */
void cleanup(void);			/* cleanup function for the test */

struct test_case_t {		/* test case struct. to hold diff. test.conds */
	int flag;
	int size;
	char *desc;
	int exp_errno;
} Test_cases[] = {
	{
	INVAL_FLAGS, SIGSTKSZ, "Invalid Flag value", EINVAL},
	/* use value low enough for all kernel versions
	 * avoid using MINSIGSTKSZ defined by glibc as it could be different
	 * from the one in kernel ABI
	 */
	{
	0, (2048 - 1), "alternate stack is < MINSIGSTKSZ", ENOMEM},
	{
	0, 0, NULL, 0}
};

int main(int ac, char **av)
{
	int lc;
	char *test_desc;	/* test specific error message */
	int ind;		/* counter to test different test conditions */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			sigstk.ss_size = Test_cases[ind].size;
			sigstk.ss_flags = Test_cases[ind].flag;
			test_desc = Test_cases[ind].desc;

			/* Verify sigaltstack() fails and sets errno */
			TEST(sigaltstack(&sigstk, NULL));

			/* Check return code from sigaltstack() */
			if (TEST_RETURN == -1) {
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
						 Test_cases
						 [ind].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "sigaltstack() returned %ld, "
					 "expected -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}
		tst_count++;	/* incr. TEST_LOOP counter */
	}

	cleanup();
	tst_exit();

}

/*
 * void
 * setup() - performs all ONE TIME setup for this test.
 * Allocate memory for the alternative stack.
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Allocate memory for the alternate stack */
	if ((sigstk.ss_sp = malloc(SIGSTKSZ)) == NULL) {
		tst_brkm(TFAIL, cleanup,
			 "could not allocate memory for the alternate stack");
	}
}

/*
 * void
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  Free the memory allocated for alternate stack.
 */
void cleanup(void)
{

	free(sigstk.ss_sp);

}
