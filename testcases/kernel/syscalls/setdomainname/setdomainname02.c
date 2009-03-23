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
 *    TEST IDENTIFIER   : setdomainname02
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : test for checking error conditions for setdomainame(2)
 *
 *    TEST CASE TOTAL   : 3
 *
 *    AUTHOR            : Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 * DESCRIPTION
 * 	Verify that,
 *   1) setdomainname(2) returns -1 and sets errno to EINVAL if the parameter,
 *	len is less than zero
 *   2) setdomainname(2) returns -1 and sets errno to EINVAL if value of
 *	len is greater than the maximum allowed value
 *   3) setdomainname(2) returns -1 and sets errno to EFAULT for a bad address
 *	for name
 *
 * ALGORITHM
 * Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *   Save current domainname
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
 *        Restore old domain name.
 *        Print errno log and/or timing stats if options given
 *  Side Effects :
 *	 setdomainname() is resetting value to NULL, if an invalid address
 *	 is given for name. So, to overcome this problem, domainname is
 *	 resetting to original value as part of cleanup() routine.
 *
 * USAGE:  <for command-line>
 *  setdomainname02  [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sys/utsname.h>

#define MAX_NAME_LEN _UTSNAME_DOMAIN_LENGTH - 1

static void cleanup(void);
static void setup(void);

char *TCID = "setdomainname02";
int TST_TOTAL = 3;
extern int Tst_count;

static int exp_enos[] = { EINVAL, EFAULT, 0 };	/* 0 terminated list of *
						 * expected errnos */

static char old_domain_name[MAX_NAME_LEN];
static struct test_case_t {
	char *desc;
	char *name;
	int len;
	int exp_errno;
	char err_desc[10];
} test_cases[] = {
	{
	"test with len = -1", "test_dom", -1, EINVAL, "EINVAL"}, {
	"test with len > allowed maximum", "test_dom", MAX_NAME_LEN + 1,
		    EINVAL, "EINVAL"}, {
"test with name = NULL", NULL, MAX_NAME_LEN, EFAULT, "EFAULT"},};

int main(int ac, char **av)
{
	int lc, ind;		/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {

			/*
			 * call the system call with the TEST() macro
			 */
			TEST(setdomainname(test_cases[ind].name,
					   (size_t) test_cases[ind].len));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == test_cases[ind].exp_errno)) {
				tst_resm(TPASS, "expected failure; Got %s",
					 test_cases[ind].err_desc);
			} else {
				tst_resm(TFAIL, "Call failed to produce "
					 "expected error;  Expected errno: %d "
					 "Got : %d, %s",
					 test_cases[ind].exp_errno,
					 TEST_ERRNO, strerror(TEST_ERRNO));
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

	/* Check whether we are root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	/* Save current domainname */
	if ((getdomainname(old_domain_name, MAX_NAME_LEN)) < 0) {
		tst_brkm(TBROK, tst_exit, "getdomainname() failed while"
			 " getting current domain name");
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Restore domain name */
	if ((setdomainname(old_domain_name, sizeof(old_domain_name)))
	    < 0) {
		tst_resm(TWARN, "setdomainname() failed while restoring"
			 " domainname to \"%s\"", old_domain_name);
	}

	/* exit with return code appropriate for results */
	tst_exit();
}
