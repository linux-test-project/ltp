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
 *    TEST IDENTIFIER	: sysfs04
 *
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Test checking for basic error conditions
 *				 for sysfs(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This test case checks whether sysfs(2) system call  returns
 *	appropriate error number for invalid
 *	option.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	  Loop if the proper options are given.
 *	  Execute system call with invaid  option parameter
 *
 *	  Check return code, if system call fails with errno == expected errno
 *		Issue syscall passed with expected errno
 *	  Otherwise,
 *	  Issue syscall failed to produce expected errno
 *
 *	Cleanup:
 *	  Do cleanup for the test.
 *
 * USAGE:  <for command-line>
 * sysfs04  [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 * where:
 *	-c n : run n copies simultaneously
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS:
 *No libc or glibc support
 *****************************************************************************/

#include <errno.h>
#include  <syscall.h>
#include "test.h"
#include "usctest.h"

#define INVALID_OPTION 100
static void setup();
static void cleanup();

char *TCID = "sysfs04";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
static int exp_enos[] = { EINVAL, 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

#ifdef __NR_sysfs

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;
		TEST(syscall(__NR_sysfs, INVALID_OPTION));

		/* check return code */
		if ((TEST_RETURN == -1) && (TEST_ERRNO == EINVAL)) {
			tst_resm(TPASS, "sysfs(2) expected failure;"
				 " Got errno - EINVAL :" " Invalid option");
		} else {
			tst_resm(TFAIL, "sysfs(2) failed to produce"
				 " expected error; %d, errno"
				 " : EINVAL and got %d", EINVAL, TEST_ERRNO);
		}

		TEST_ERROR_LOG(TEST_ERRNO);
	}
#else
	tst_resm(TWARN,
		 "This test can only run on kernels that support the sysfs system call");
#endif

	/*Clean up and exit */
	cleanup();
	tst_exit();

}				/*End of main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/*set the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

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

}