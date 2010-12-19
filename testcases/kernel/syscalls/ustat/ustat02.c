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
 *    TEST IDENTIFIER	: ustat02
 *
 *
 *    EXECUTED BY	: Anyone
 *
 *    TEST TITLE	: Test checking for basic error conditions
 *    				 for ustat(2)
 *
 *    TEST CASE TOTAL	: 2
 *       $
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This test case checks whether ustat(2) system call  returns
 *	appropriate error number for invalid
 *	dev_t parameter. Next, it checks for bad address paramater.
 *
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  For testing error on invalid parameter, set dev_num to -1
 *
 * 	Test:
 *	  Loop if the proper options are given.
 *	  Execute system call with invaid flag parameter
 *	  and then for invalid user
 *	  Check return code, if system call fails with errno == expected errno
 *		Issue syscall passed with expected errno
 *	  Otherwise,
 *	  Issue syscall failed to produce expected errno
 *
 * 	Cleanup:
 * 	  Do cleanup for the test.
 * 	 $
 * USAGE:  <for command-line>
 *  ustat02 [-c n] [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 *  where
 *  	-c n: run n copies simultaneously
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *
 *RESTRICTIONS: None
 *****************************************************************************/
#include <errno.h>
#include "test.h"
#include "usctest.h"
#include <sys/types.h>
#include <unistd.h>		/* libc[45] */
#include <ustat.h>		/* glibc2 */
#include <sys/stat.h>

static void setup();
static void cleanup();

char *TCID = "ustat02";		/* Test program identifier.    */

static int exp_enos[] = { EINVAL, EFAULT, 0 };

static struct test_case_t {
	char *err_desc;		/*error description */
	int exp_errno;		/* expected error number */
	char *exp_errval;	/*Expected errorvalue string */
} testcase[] = {
	{
	"Invalid parameter", EINVAL, "EINVAL"},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	{
	"Bad address", EFAULT, "EFAULT"}
#endif
};

int TST_TOTAL = sizeof(testcase) / sizeof(*testcase);	/* Total number of test cases. */

dev_t dev_num[2];
struct ustat *ubuf;
struct stat *buf;

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

		for (i = 0; i < TST_TOTAL; i++) {
			if (i == 0) {
				TEST(ustat(dev_num[i], ubuf));
			} else {
				TEST(ustat(dev_num[i], (struct ustat *)-1));
			}

			if ((TEST_RETURN == -1) && (TEST_ERRNO == testcase[i].
						    exp_errno)) {
				tst_resm(TPASS, "ustat(2) expected failure;"
					 " Got errno - %s : %s",
					 testcase[i].exp_errval,
					 testcase[i].err_desc);
			} else {
				tst_resm(TFAIL, "ustat(2) failed to produce"
					 " expected error; %d, errno"
					 ": %s and got %d",
					 testcase[i].exp_errno,
					 testcase[i].exp_errval, TEST_ERRNO);
			}

			TEST_ERROR_LOG(TEST_ERRNO);
		}		/*End of TEST LOOPS */
	}

	/*Clean up and exit */
	cleanup();

	tst_exit();
}				/*End of main */

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	dev_num[0] = -1;

	/* Allocating memory for ustat and stat structure variables */
	if ((ubuf = (struct ustat *)malloc(sizeof(struct ustat))) == NULL) {
		tst_brkm(TBROK, NULL, "Failed to allocate Memory");
	}

	if ((buf = (struct stat *)malloc(sizeof(struct stat))) == NULL) {
		free(ubuf);
		tst_brkm(TBROK, NULL, "Failed to allocate Memory");
	}

	/* Finding out a valid device number */
	if (stat("/", buf) != 0) {
		free(buf);
		free(ubuf);
		tst_brkm(TBROK, NULL, "stat(2) failed. Exiting without"
			 "invoking ustat(2)");
	}
	dev_num[1] = buf->st_dev;
}

/*
* cleanup() - Performs one time cleanup for this test at
* completion or premature exit
*/
void cleanup()
{
	free(ubuf);
	free(buf);
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}