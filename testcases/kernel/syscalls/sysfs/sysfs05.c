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
/**************************************************************************
 *
 *    TEST IDENTIFIER	: sysfs(2)
 *
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Test checking for basic error conditions
 *				 for sysfs(2)
 *
 *    TEST CASE TOTAL	: 3
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This test case checks whether sysfs(2) system call returns
 *	appropriate error number for invalid
 *	option and for invalid filesystem name.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	  Loop if the proper options are given.
 *	  Execute system call with invaid option parameter and for
 *	  invalid filesystem name
 *	  Check return code, if system call fails with errno == expected errno
 *		Issue syscall passed with expected errno
 *	  Otherwise,
 *	  Issue syscall failed to produce expected errno
 *
 *	Cleanup:
 *	  Do cleanup for the test.
 *
 * USAGE:  <for command-line>
 *  sysfs05 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-f] [-h] [-p]
 *  where:
 *	-c n : Run n copies simultaneously
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 *There is no libc or glibc support
 *Kernel must be compiled with ext2 support
 *****************************************************************************/

#include <errno.h>
#include <syscall.h>
#include "test.h"
#include "usctest.h"

static void setup();
static void cleanup();

char *TCID = "sysfs05";		/* Test program identifier.    */
extern int Tst_count;		/* Test Case counter for tst_* routines */
static int option[3] = { 1, 4, 1 };	/* valid and invalid option */
static char *fsname[] = { "ext0", " ext2", (char *)-1 };
static int exp_enos[] = { EINVAL, EFAULT, 0 };

static struct test_case_t {
	char *err_desc;		/*error description */
	int exp_errno;		/* expected error number */
	char *exp_errval;	/*Expected errorvalue string */
} testcase[] = {
	{
	"Invalid option", EINVAL, "EINVAL"}, {
	"Invalid filesystem name", EINVAL, "EINVAL "}, {
	"Address is out of your address space", EFAULT, "EFAULT "}
};
int TST_TOTAL = sizeof(testcase) / sizeof(*testcase);

int main(int ac, char **av)
{
	int lc, i;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != NULL)
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);

	/* perform global setup for test */
	setup();

#ifdef __NR_sysfs
	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		for (i = 0; i < TST_TOTAL; i++) {

			/* reset Tst_count in case we are looping. */
			Tst_count = 0;
			TEST(syscall(__NR_sysfs, option[i], fsname[i]));

			/* check return code */
			if ((TEST_RETURN == -1)
			    && (TEST_ERRNO == testcase[i].exp_errno)) {
				tst_resm(TPASS,
					 "sysfs(2) expected failure;"
					 " Got errno - %s : %s",
					 testcase[i].exp_errval,
					 testcase[i].err_desc);
			} else {
				tst_resm(TFAIL, "sysfs(2) failed to produce"
					 " expected error; %d, errno"
					 ": %s and got %d",
					 testcase[i].exp_errno,
					 testcase[i].exp_errval, TEST_ERRNO);
			}
			TEST_ERROR_LOG(TEST_ERRNO);

		}		/*End of TEST LOOPS */
	}			/* End of TEST_LOOPING */
#else
	tst_resm(TWARN,
		 "This test can only run on kernels that support the sysfs system call");
#endif

	/*Clean up and exit */
	cleanup();

	return 0;
}				/*End of main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* set the expected erronos */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;
}				/* End setup() */

/*
* cleanup() - Performs one time cleanup for this test at
* completion or premature exit
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
}				/* End cleanup() */
